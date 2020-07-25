#ifndef _EasyEpollServer_hpp_
#define _EasyEpollServer_hpp_

#include"EasyTcpServer.hpp"
#include"CELLEpollServer.hpp"
#include"../share/CELLEpoll.hpp"

//
class EasyEpollServer : public EasyTcpServer {
public:
	void Start(int nCELLServer) {
		EasyTcpServer::Start<CELLEpollServer>(nCELLServer);
	}
	void OnRun(CELLThread* pThread)
	{
		CELLEpoll ep;
		ep.create(1);
		ep.ctl(EPOLL_CTL_ADD, sockfd(), EPOLLIN);
		while (pThread->isRun())
		{
			time4msg();
			int ret = ep.wait(1);
			if(ret < 0) {			
				Log::INFO("EasyTcpServer.OnRun select exit.\n");
				pThread->Exit();
				break;
			}
			auto events = ep.events();
			for(int i = 0; i < ret; i ++) {
				if(events[i].data.fd == sockfd()) {
					Accept();
				}
			}
		}
	}
};

#endif // !_EasyTcpServer_hpp_
