#include "./EasyEpollServer.hpp"
#include "../share/CELL.hpp"


int main(int argc, char* args[]) {

	Log::GetLogInstance()->init("./serverLog",100000);
	Log::INFO("--------------------server is start--------------------\n");
	if (argc < 5) {
		printf("main parameters error \n");
		Log::ERROR("main parameters error \n");
	}
		
	const char* strIP = args[1];
	uint16_t nPort = (uint16_t)std::stoi(args[2]);
	int nThread = std::stoi(args[3]);
	int nClient = std::stoi(args[4]);
	
	if (0 == strcmp(strIP, "any")) strIP = nullptr; //代表监控主机所有ip
	EasyEpollServer server;
	server.setMaxClient(nClient);
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(10000);
	server.Start(nThread);

	//在主线程中等待用户输入命令
	
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			server.Close();
			break;
		}
		else {
			Log::WARN("undefine cmd\n");
		}
	}

	Log::INFO("--------------------server is finish--------------------\n");

	return 0;
}
