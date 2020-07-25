#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include"../share/CELL.hpp"

/*
	封装读写缓冲区：包含一个缓冲Buff, 支持同步recv, send套接口，以及处理分包，粘包
*/

class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	} 

	//向写缓冲区写入一条完整的消息
	bool push(const char* pData, int nLen)
	{
		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SZIE)
			{
				++_fullCount;
			}

			return true;
		}
		else {
			++_fullCount;
		}

		return false;
	}

	//弹出读缓冲区的第一条完整的消息
	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}

	//写
	int write2socket(SOCKET sockfd)
	{
		int ret = 0;
		//缓冲区有数据
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _nLast, 0);
			if (ret <= 0) {
				return SOCKET_ERROR;
			}
			if (ret == _nLast) {
				//数据尾部位置清零
				_nLast = 0;
			}
			else {
				_nLast -= ret;
				memcpy(_pBuff, _pBuff + ret, _nLast);
			}
			_fullCount = 0;
		}
		return ret;
	}

	//读
	int read4socket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen <= 0)
			{
				return SOCKET_ERROR;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	//判断客户端是否接受到一条完整的消息
	bool hasMsg()
	{
		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			//这时就可以知道当前消息的长度
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			//判断消息缓冲区的数据长度大于消息长度
			return _nLast >= header->dataLength;
		}
		return false;
	}

	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
};

#endif // !_CELL_BUFFER_HPP_
