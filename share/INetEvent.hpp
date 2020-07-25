#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

//自定义
class CELLClient;

//网络事件接口
class INetEvent
{
public:
	//客户端加入事件
	virtual void OnNetJoin(CELLClient* pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave(CELLClient* pClient) = 0;
	//接受到完整的客户端消息的次数
	virtual void OnNetMsg(CELLClient* pClient) = 0;
	//Recv的次数
	virtual void OnNetRecv(CELLClient* pClient) = 0;
};

#endif // !_I_NET_EVENT_HPP_
