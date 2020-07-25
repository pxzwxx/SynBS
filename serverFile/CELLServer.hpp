#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"../share/CELL.hpp"
#include"../share/CELLClient.hpp"
#include"../share/CELLThread.hpp"

//网络消息接收处理服务类
class CELLServer {
public:
	CELLServer() {
		_pNetEvent = nullptr;
	}

	virtual	~CELLServer() {
		Log::INFO("CELLServer%d.~CELLServer exit begin\n", _id);
		Close();
		Log::INFO("CELLServer%d.~CELLServer exit end\n", _id);
	}
	
	void setId(int id) {
		_id = id;
	}
	
	//纯虚函数，子类必须重写, 设置IO复用模型可以监听的文件描述符个数	
	virtual void setIOListenNum(int nSocketNum) = 0;
	//纯虚函数，子类必须重写, 设置新客户端加入后的IO复用模型的添加事件方式
	virtual void addClient2IO(CELLClient *pClient) = 0;
	//纯虚函数，子类必须重写, 网络读写事件的重写
	virtual bool DoNetEvents() = 0;	
	//纯虚函数，处理网络消息	
	virtual void ProcessNetMsg(CELLClient* pClient, netmsg_DataHeader* header) = 0;
	//移除IO复用模型中的文件描述符号
	virtual void removeIO(CELLClient* pClient) = 0;	

	void setEventObj(INetEvent* event) {
		_pNetEvent = event;
	}

	//关闭Socket
	void Close() {
		Log::INFO("CELLServer%d.Close begin\n", _id);
		_thread.Close();
		Log::INFO("CELLServer%d.Close end\n", _id);
	}
	
	//处理网络消息
	void OnRun(CELLThread* pThread) {
		while (pThread->isRun()) {
			if (!_clientSocketBuff.empty()) {//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto cSock : _clientSocketBuff) {
					CELLClient *pClient = new CELLClient(cSock);
					_clients[cSock] = pClient;
					pClient->serverId = _id;

					if (_pNetEvent)
						_pNetEvent->OnNetJoin(_clients[cSock]);
					addClient2IO(_clients[cSock]);
				}
				_clientSocketBuff.clear();
			}
			//如果没有需要处理的客户端，就跳过
			if (_clients.empty()) {
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			if (!DoNetEvents()) { //读写socket
				pThread->Exit();
				break;
			}
			DoMsg(); //处理时间，分包，组织响应消息
			processTimer(); //处理定时器心跳时间
		}
		Log::INFO("CELLServer%d.OnRun exit\n", _id);
	}
	
	//检查并处理总定时器超时	
	void processTimer() { //心跳检测
		time_t cur = time(NULL);
		for(auto &client : _clients) {
			auto pClient = client.second;
			if(cur >= pClient->_expire) {
				rmClient(pClient);
			}
		}
	}	

	//处理缓冲区的消息
	void DoMsg() { 
		for (auto iter : _clients) {
			auto pClient = iter.second;
			//接收客户端数据
			while (pClient->hasMsg()) {
				ProcessNetMsg(pClient, pClient->front_msg()); //处理网络消息
				pClient->pop_front_msg(); //移除消息队列（缓冲区）最前的一条数据
				_pNetEvent->OnNetMsg(pClient);//添加消息计数 
			}
		}
	}
	
	//清除客户端
    void rmClient(CELLClient* pClient) {
		removeIO(pClient); //移除IO复用中的文件描述符
        auto iter = _clients.find(pClient->sockfd());
        if(iter != _clients.end()) {
			if(pClient) {
				if(pClient->sockfd() > 0) {
					close(pClient->sockfd()); //关闭文件描述符
				}
				if(pClient) {
					delete pClient;
				}
			}
			_clients.erase(iter);
        }
        if(_pNetEvent) {
            _pNetEvent->OnNetLeave(pClient);
        }
    }
	
	//添加客户端
	void addClient(SOCKET sock) {
		std::lock_guard<std::mutex> lock(_mutex);
		_clientSocketBuff.push_back(sock);
	}

	//开启服务线程
	void Start() {
		_thread.Start(
			nullptr,
			[this](CELLThread* pThread) {
				OnRun(pThread);
			},
			[this](CELLThread* pThread) {
				ClearClients();
			}
		);
	}
	
	//获取总客户端数
	size_t getClientCount() {
		return _clients.size() + _clientSocketBuff.size();
	}

private:
	void ClearClients() {
		for (auto iter : _clients) {
			delete iter.second;
		}
		_clients.clear();
		_clientSocketBuff.clear();
	}
protected:
	//正式客户队列
	std::map<SOCKET, CELLClient*> _clients;
	//客户列表是否有变化
	int _id = -1;
	//网络事件对象
	INetEvent* _pNetEvent;
private:
	//缓冲客户队列
	std::vector<SOCKET> _clientSocketBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//任务线程句柄
	CELLThread _thread;
};

#endif // !_CELL_SERVER_HPP_
