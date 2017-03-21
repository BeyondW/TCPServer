#include <iostream>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include "Message.h"
#include <iomanip>      // std::put_time
#include <ctime>		// std::time_t, struct std::tm, std::localtime
#pragma comment(lib,"ws2_32.lib")
using namespace std;

typedef std::chrono::system_clock::time_point TimePoint;
std::mutex stdLock;
std::string msgBuff = "";
unsigned int clientId = 0;

void getMsg(SOCKET clientSocket)
{
	char buf[1024];
	int iResult;
	while (true)
	{
		iResult = recv(clientSocket, buf, 1024, 0);
		if (iResult <= 0)
		{
			break;
		}
		char str[1024] = "";
		unsigned int length;
		if (!splitPacket(buf, str, msgBuff, length))
		{
			continue;
		}
		/*stdLock.lock();
		std::cout << "recvMsg:get msg" << std::endl;
		stdLock.unlock();*/
		//类型判断
		switch (getType(str))
		{
		case HEARTBEAT:
		{
			Msg hbMsg;
			deserialize(&hbMsg, str, length);
		/*	TimePoint curTime = std::chrono::system_clock::now();
			auto t = std::chrono::system_clock::to_time_t(curTime);
			stdLock.lock();
			std::cout << "Server active time:" << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << std::endl;
			stdLock.unlock();*/
			clientId = hbMsg.clientId;
			char classBuff[1024];
			serialize(classBuff, &hbMsg, sizeof(hbMsg));
			std::string msgStr(classBuff, sizeof(hbMsg));
			std::string buffer = producePacket(msgStr);
			send(clientSocket, buffer.c_str(), buffer.length(), 0);
			/*stdLock.lock();
			std::cout << "heartBeat:send hb|" << std::endl;
			stdLock.unlock();*/

			break;
		}
		case CHATMSG:
		{
			ChatMsg msg;
			deserialize(&msg, str, length);
			stdLock.lock();
			std::cout << msg.clientId << ":" << msg.content << endl;
			stdLock.unlock();
			break;
		}
		default:
			break;
		}
	}

}

void rsnonBlockExample(const SOCKET& clientSocket)
{
	thread t(getMsg, clientSocket);
	t.detach();
	std::string str;
	char buf[1024];
	int iResult;
	while (1)
	{
		cin >> str;
		produceChatMsg(buf, str, clientId);
		std::string msgStr(buf, sizeof(ChatMsg));
		std::string buffer = producePacket(msgStr);
		iResult = send(clientSocket, buffer.c_str(), length, 0);
	}
}



int main()
{
	
	
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);	//初始化WS2_32.DLL

	SOCKET clientSocket;
	//创建套接字
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9102);
	serveraddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	connect(clientSocket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	cout << "连接成功!" << endl;

	//发送数据
	cout << "请输入发送给服务器的字符:" << endl;

	rsnonBlockExample(clientSocket);

	//接收数据
	
	//关闭套接字
	closesocket(clientSocket);
	WSACleanup();    //释放WS2_32.DLL
	return 0;
}