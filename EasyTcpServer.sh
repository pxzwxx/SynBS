strIP="127.0.0.1" #绑定ip
nPort=4567		  #绑定端口
nThread=6		  #服务端线程池的线程数		
nClient=80000     #服务端支持最大接入客户端数量
./server $strIP $nPort $nThread $nClient
