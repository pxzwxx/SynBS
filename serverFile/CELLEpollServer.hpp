#ifndef _CELL_EPOLL_SERVER_HPP_
#define _CELL_EPOLL_SERVER_HPP_

#include"CELLServer.hpp"
#include"../share/CELLEpoll.hpp"
#include"../share/CELL.hpp"


/*
	每个子服务类同步recv, 处理网络数据， send　
*/

class CELLEpollServer : public CELLServer {
public:
	~CELLEpollServer() {
		Close();
	}	
	CELLEpollServer() {}
	
	//重写父亲的方法
	void setIOListenNum(int nSocketNum) {
		_ep.create(nSocketNum);
	}	
	
	//重写父类的方法	
	void addClient2IO(CELLClient* pClient) {
		_ep.ctl(EPOLL_CTL_ADD, pClient, EPOLLIN);
	}	

	//重写父类的方法	
	void removeIO(CELLClient* pClient) {
		_ep.del(pClient);
	}	
	
	//重写父类的方法
	bool DoNetEvents() {
		int ret = _ep.wait(1);
		if(ret < 0) {
			strerror(errno); //print the reason of wrong
			Log::INFO("CELLEpollServer DoNetEvents error\n");
			return false;
		}
		else if(ret == 0) return true;
		else {
			auto events = _ep.events();
			for(int i = 0; i < ret; i ++) {
				CELLClient *pClient = (CELLClient*)events[i].data.ptr;
				if(pClient) {
					if(events[i].events & EPOLLIN) {
						if(SOCKET_ERROR == pClient->RecvData()) {
							rmClient(pClient);
							continue;
						}
						else { //接收消息成功, 接收计数++
							_pNetEvent->OnNetRecv(pClient);
						}	
					}
					if(events[i].events & EPOLLOUT) {
						if(SOCKET_ERROR == pClient->SendDataReal()) {
							rmClient(pClient);
							continue;
						}
						else { //修改当前描述符号为只读
							_ep.ctl(EPOLL_CTL_MOD, pClient, EPOLLIN);
						}	
					}
				} 
			}
		}				
		return true;
	}

	//网络处理函数
    virtual void ProcessNetMsg(CELLClient* pClient, netmsg_DataHeader* header) {
        switch (header->cmd) {
            case CMD_LOGIN: {
                netmsg_Login* login = (netmsg_Login*)header;
                if (_bCheckMsgID) {
                    if (login->msgID != pClient->_nRecvMsgID) {
                        printf("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d>\n",pClient->sockfd(), login->msgID, pClient->_nRecvMsgID);
						Log::INFO("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d>\n",pClient->sockfd(), login->msgID, pClient->_nRecvMsgID);("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d>\n",pClient->sockfd(), login->msgID, pClient->_nRecvMsgID);
                    }
                    ++pClient->_nRecvMsgID;
                }
                if (_bSendBack) {
                    netmsg_LoginR ret;
                    ret.msgID = pClient->_nSendMsgID;
                    if (SOCKET_ERROR == pClient->SendData2Buffer(&ret)) {
                        if (_bSendFull) {
                            printf("<socket=%d> Send Full\n", pClient->sockfd());
							Log::INFO("<socket=%d> Send Full\n", pClient->sockfd());
                        }
                    }
                    else {
						_ep.ctl(EPOLL_CTL_MOD, pClient, EPOLLIN | EPOLLOUT); //有消息可以写，修改文件描述符可写
                        ++pClient->_nSendMsgID;
                    }
                }
            }break;

            case CMD_LOGOUT: {
                netmsg_Logout* logout = (netmsg_Logout*)header;
                Log::INFO("recv <Socket=%d> msgType：CMD_LOGOUT, dataLen：%d,userName=%s \n", pClient->sockfd(), logout->dataLength, logout->userName);
            }break;

            case CMD_C2S_HEART: {
                netmsg_s2c_Heart ret;
                pClient->SendData2Buffer(&ret);
            }

            default: {
                Log::WARN("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
            }break;

        }
    }

public:
	//消息检查标志位
    bool _bCheckMsgID = true;
    bool _bSendBack = true;
    bool _bSendFull = true;
private:
	CELLEpoll _ep;
	SOCKET _maxSock;
};

#endif // !_CELL_EPOLL_SERVER_HPP_
