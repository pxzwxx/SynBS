#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"../share/CELL.hpp"
#include"../share/CELLClient.hpp"
#include"../share/INetEvent.hpp"
#include"CELLServer.hpp"
#include"CELLEpollServer.hpp"

class EasyTcpServer : public INetEvent {
private:
	//
	CELLThread _thread;
	//消息处理对象，内部会创建线程
	std::vector<CELLServer*> _cellServers;

	time_t _old = time(NULL);

	SOCKET _sock;
	
	//消息检查标志位
    bool _bCheckMsgID = true;
    bool _bSendBack = true;
    bool _bSendFull = true;
protected:
	
	//客户端最大连接上限
	int _nMaxClient;	
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 1;
		_clientCount = 0;
		_nMaxClient = 0;
	}
	void setMaxClient(int maxClient) {
		_nMaxClient = maxClient; 
	}
	virtual ~EasyTcpServer() {
		Close();
	}
	//初始化Socket
	SOCKET InitSocket() {
		if (INVALID_SOCKET != _sock) {
			Log::ERROR("warning, initSocket close old socket<%d>...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			Log::ERROR("error, create socket failed...\n");
		}
		else {
			int flag = 1;
			if(SOCKET_ERROR == setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int))) {
				Log::INFO("setsockopt socket<%d> SO_REUSEADDR fail",(int)(_sock));
				return SOCKET_ERROR;
			}
			Log::ERROR("create socket<%d> success...\n", (int)_sock);
		}
		return _sock;
	}

	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			Log::ERROR("error, bind port<%d> failed...\n", port);
		}
		else {
			Log::INFO("bind port<%d> success...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n) {
		// 3 listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			Log::ERROR("error, listen socket<%d> failed...\n",_sock);
		}
		else {
			Log::INFO("listen port<%d> success...\n", _sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept() {
		// 4 accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
		if (INVALID_SOCKET == cSock) {
			Log::INFO("error, accept INVALID_SOCKET...\n");
		}
		else {
			//将新客户端分配给客户数量最少的cellServer
			addClientToCELLServer(cSock);
		}
		return cSock;
	}
	
	void addClientToCELLServer(SOCKET cSock) {
		//查找客户数量最少的CELLServer消息处理对象
		auto pMinServer = _cellServers[0];
		for(auto pServer : _cellServers) {
			if (pMinServer->getClientCount() > pServer->getClientCount()) {
				pMinServer = pServer;
			}
		}
		pMinServer->addClient(cSock);
	}
	
	template<typename ServerT>
	void Start(int nCELLServer) {
		for (int n = 0; n < nCELLServer; n++) {
			auto ser = new ServerT;
			ser->setId(n+1);
			ser->setIOListenNum((_nMaxClient / nCELLServer) + 1);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
		_thread.Start(
			nullptr,
			[this](CELLThread* pThread) {
				OnRun(pThread);
			},
			nullptr
		);
	}

	//关闭Socket
	void Close()
	{
		Log::INFO("EasyTcpServer.Close begin\n");
		_thread.Close();
		if (_sock != INVALID_SOCKET) {
			for (auto s : _cellServers) {
				delete s;
			}
			_cellServers.clear();
			close(_sock);
			_sock = INVALID_SOCKET;
		}
		Log::INFO("EasyTcpServer.Close end\n");
	}
	
	//虚函数重写	
	virtual void OnNetJoin(CELLClient* pClient) {
		_clientCount++;
		Log::INFO("client<%d> OnNetjoin\n", pClient->sockfd());
	}
	
	//虚函数重写
	virtual void OnNetLeave(CELLClient* pClient) {
		_clientCount--;
		Log::INFO("client<%d> OnNetleave\n", pClient->sockfd());
	}

	//虚函数重写
	virtual void OnNetRecv(CELLClient* pClient) {
		_recvCount++;
	}
	
	//虚函数重写
    virtual void OnNetMsg(CELLClient* pClient) {
		_msgCount++;
    }


protected:
	SOCKET sockfd() {
		return _sock;
	}
	
	virtual void OnRun(CELLThread *pThread) = 0;

	//计算并输出每秒收到的网络消息
	void time4msg() {
		time_t cur = time(NULL);
		int t1 = cur - _old;
		if (t1 >= 1) {
			_old = cur;
			Log::INFO("thread<%d>,time<%d>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount), (int)(_msgCount));	
			printf("thread<%d>,time<%d>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount), (int)(_msgCount));
			_recvCount = 0;
			_msgCount = 0;
		}
	}
};

#endif // !_EasyTcpServer_hpp_
