#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include"../share/CELL.hpp"
#include"./CELLBuffer.hpp"

//客户端心跳检测死亡计时时间
#define CLIENT_HREAT_DEAD_TIME 60000 * 5
//在间隔指定时间后
//把发送缓冲区内缓存的消息数据发送给客户端
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据类型

class CELLClient
{
public:
	int id = -1;
	//所属serverid
	int serverId = -1;
	bool _needWrite;
public:
	CELLClient(SOCKET sockfd = INVALID_SOCKET, int sendBuffSize = SEND_BUFF_SZIE, int recvBuffSize = RECV_BUFF_SZIE) :_recvBuff(recvBuffSize), _sendBuff(sendBuffSize)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
		_expire = time(NULL) + TIMESLOT;
		resetDTSend();
	}

	~CELLClient()
	{
		Log::INFO("s=%d CELLClient%d.~CELLClient\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
			close(_sockfd);
			_sockfd = INVALID_SOCKET;
		}
	}
	
	SOCKET sockfd()
	{
		return _sockfd;
	}
	//协程异步读socket
	int RecvData()
	{
		_expire = time(NULL) + 1 * TIMESLOT;
		return _recvBuff.read4socket(_sockfd);
	}
	//判断是否接收到完整的消息
	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}
	//获取第一条消息
	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if(hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDTSend();
		_expire = time(NULL) + 1 * TIMESLOT;
		return _sendBuff.write2socket(_sockfd);
	}

	//发送数据到缓冲区
	int SendData2Buffer(netmsg_DataHeader* header)
	{
		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}
	
	//定时发送
	void resetDTSend()
	{
		_dtSend = 0;
	}

	bool needWrite() {
		return _sendBuff.needWrite();
	}

private:
	SOCKET _sockfd;
	//第二缓冲区 接收消息缓冲区
	CELLBuffer _recvBuff;
	//发送缓冲区
	CELLBuffer _sendBuff;
	//上次发送消息数据的时间 
	time_t _dtSend;
	//发送缓冲区遇到写满情况计数
	int _sendBuffFullCount = 0;

public:
	int _nRecvMsgID = 1;
	int _nSendMsgID = 1;
	int _nSendCount = 0;
	//心跳时间
	time_t _expire;
};

#endif // !_CELLClient_HPP_
