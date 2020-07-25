#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET
#include<unistd.h> 
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<errno.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define EPOLL_ERROR (-1)

#include"MessageHeader.hpp"
#include"INetEvent.hpp"
#include"CELLThread.hpp"
#include"lock.h"
#include"log.h"
#include"timer.h"
#include<sys/epoll.h>
#include<stdio.h>
#include<iostream>
#include<thread>
#include<string>
#include<vector>
#include<map>
#include<mutex>
#include<atomic>


//缓冲区最小单元大小
#define RECV_BUFF_SZIE 1024 * 5
#define SEND_BUFF_SZIE 1024 * 5 
#define CLIENT_SEND_BUFF_TIME 200
#define TIMESLOT 60 * 3 //超时10s

#endif // !_CELL_HPP_
