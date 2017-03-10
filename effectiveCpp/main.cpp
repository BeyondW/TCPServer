//TCP server  
//listen port 9102  
//receive string and display it  

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <iomanip>      // std::put_time
#include <ctime>		// std::time_t, struct std::tm, std::localtime
#include <memory>		// std::shared_ptr
#include <atomic>
#include <WinSock2.h> 
#include "Client.h"

#pragma comment(lib,"Ws2_32.lib")  


typedef std::shared_ptr<Client> ClientPtr;

std::mutex vLock;
std::mutex stdLock;
std::mutex timeLock;
std::vector<ClientPtr> clientList;
TimePoint lastTestTime;
std::atomic<unsigned int> curClientId = 0;
std::string msgBuff = "";

bool isOverStringBound(std::size_t pos, const std::string& str)
{
	return pos > str.length() ? true : false;
	
}

//拆包函数
bool splitPacket(const char* buff, std::string& str)
{
	msgBuff += buff;
	auto pos = msgBuff.find("@HEAD");
	if (pos != std::string::npos)
	{
		if (!isOverStringBound(pos + 5, msgBuff))
		{
			int msgLength = msgBuff[pos + 5];
			if (!isOverStringBound(pos + 5 + msgLength, msgBuff))
			{
				str = msgBuff.substr(pos + 5, msgLength);
				msgBuff.erase(0, pos + 5 + msgLength);
				return true;
			}
		}

	}
	return false;
}

//to do 组包函数
std::string producePacket(std::string& msg)
{
	std::string packet = "@HEAD" + msg.length() + msg;
	return packet;
}

void heartBeat()
{
	while (true)
	{	
		vLock.lock();
		if (clientList.size())
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			//send hb msg
			char buf[2] = "h";
			for (auto clientPtr : clientList)
			{
				send(clientPtr->getSocket(), buf, strlen(buf) + 1, 0);
				stdLock.lock();
				std::cout << "heartBeat:send hb|" << std::endl;
				stdLock.unlock();
			}

			
			//test active clients
			for (auto it = clientList.begin(); it != clientList.end();)
			{
				timeLock.lock();
				TimePoint curTime = std::chrono::steady_clock::now();
				int activeDuration = std::chrono::duration_cast<std::chrono::seconds>(curTime - it->get()->getActiveTime()).count();
				timeLock.unlock();
				if (activeDuration > 5)
				{
					it = clientList.erase(it);//to do close socket
					std::cout << "erase client"  << std::endl;
				}
				else
				{
					it++;
				}
			}
					 
		}
		vLock.unlock();
	}

}

void recvMsg(ClientPtr clientPtr)
{
	

	while (true)
	{
		if (clientPtr.unique())
		{
			break;
		}
		char buf[1024] = "";
		recv(clientPtr->getSocket(), buf, 1024, 0);
		std::string str = "";
		if (!splitPacket(buf, str))
		{
			continue;
		}
		//if  buf = heart flag
		stdLock.lock();
		std::cout << "recvMsg:get msg" << std::endl;
		stdLock.unlock();
		if (!memcmp(str.c_str(), "h", 1))
		{
			TimePoint curTime = std::chrono::system_clock::now();
			timeLock.lock();
			clientPtr->setActiveTime(curTime);
			timeLock.unlock();
			auto t = std::chrono::system_clock::to_time_t(curTime);
			stdLock.lock();
			std::cout << "recvMsg:set active time:" << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << std::endl;
			stdLock.unlock();
		}
		else
		{
			//broadcast
			vLock.lock();
			for (auto c : clientList)
			{
				send(c->getSocket(), str.c_str(), strlen(str.c_str()) + 1, 0);
			}
			vLock.unlock();
		}
	}
}
bool validationCloseSocket(const char* msg, int iResult)
{
	if (iResult == SOCKET_ERROR)
	{
		std::cout << msg << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	else
	{
		return true;
	}
}

bool validationListenSocket(const char* msg, const SOCKET& listenSocket)
{
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << msg << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	return true;
	
}

bool validationBind(const char* msg, int iResult, const SOCKET& listenSocket)
{
	if (iResult == SOCKET_ERROR)
	{
		std::cout << msg << WSAGetLastError() << std::endl;
		int code = closesocket(listenSocket);
		validationCloseSocket("close failed with error", code);
		WSACleanup();
		return false;
	}
	else
	{
		return true;
	}

}

bool validationListen(const char* msg, int iResult, const SOCKET& listenSocket)
{
	return validationBind(msg, iResult, listenSocket);
}

bool validationAccept(const char* msg, const SOCKET& newSocket)
{
	if (newSocket == INVALID_SOCKET)
	{
		std::cout << msg << WSAGetLastError() << std::endl;
		int code = closesocket(newSocket);
		validationCloseSocket("close failed with error", code);
		WSACleanup();
		return false;
	}
	else
	{
		return true;
	}
}


int main()
{
	WSADATA wsa;
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(1, 1), &wsa); //initial Ws2_32.dll by a process  
	if (iResult != NO_ERROR) 
	{
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}
	SOCKET listenSocket = INVALID_SOCKET;
	listenSocket  = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//create a tcp socket

	if (!validationListenSocket("Error at socket():", listenSocket))
	{
		return 1;
	}
	
	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9102);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);//

	iResult = bind(listenSocket , (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (!validationBind("bind failed with error", iResult, listenSocket))
	{
		return 1;
	}
	
	iResult = listen(listenSocket, SOMAXCONN);
	if (!validationListen("listen function failed with error:", iResult, listenSocket))
	{
		return 1;
	}
	std::cout << "Start listen..." << std::endl;

	lastTestTime = std::chrono::system_clock::now();

	int len;
	len = sizeof(SOCKADDR_IN);
	SOCKET newConnection;
	//connection
	SOCKADDR_IN clientaddr;
	std::thread heartT(heartBeat);
	heartT.detach();
	while (true)
	{
		newConnection = accept(listenSocket, (SOCKADDR *)&clientaddr, &len);
		if (!validationAccept("accept failed with error:", newConnection))
		{
			continue;
		}
		curClientId++;
		auto p = new Client(newConnection, std::chrono::system_clock::now(), curClientId);
		ClientPtr cp(p);
		vLock.lock();
		clientList.push_back(cp);
		vLock.unlock();
		std::thread t(recvMsg, std::ref(cp));
		t.detach();
		stdLock.lock();
		std::cout << "New Client Connected" << "id:" << curClientId << std::endl;
		stdLock.unlock();
	}
	
    WSACleanup(); //clean up Ws2_32.dll to do :need to add validation
	return 0;
}