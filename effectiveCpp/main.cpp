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
#include "Client.h"
#include "Message.h"
#include "Validation.h"
#pragma comment(lib,"Ws2_32.lib")  


typedef std::shared_ptr<Client> ClientPtr;

std::mutex vLock;
std::mutex stdLock;
std::mutex timeLock;
std::vector<ClientPtr> clientList;
TimePoint lastTestTime;
std::atomic<unsigned int> curClientId = 0;
std::string msgBuff = "";

void heartBeat()
{
	while (true)
	{	
		vLock.lock();
		if (clientList.size())
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			//send hb msg
			Msg msg;
			msg.type = HEARTBEAT;
			for (auto clientPtr : clientList)
			{
				msg.clientId = clientPtr->getId();
				char classBuff[1024];
				serialize(classBuff, &msg, sizeof(msg));
				std::string msgStr(classBuff, sizeof(msg));
				std::string buffer = producePacket(msgStr);
				send(clientPtr->getSocket(), buffer.c_str(), buffer.length(), 0);
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
		int recvLength = recv(clientPtr->getSocket(), buf, 1024, 0);

		char str[1024] = "";
		unsigned int length;
		if (!splitPacket(buf, recvLength, str, msgBuff, length))
		{
			continue;
		}
		stdLock.lock();
		std::cout << "recvMsg:get msg" << std::endl;
		stdLock.unlock();
		//ÀàÐÍÅÐ¶Ï
		switch (getType(str))
		{
			case HEARTBEAT:
			{
				Msg hbMsg;
				deserialize(&hbMsg, str, length);
				TimePoint curTime = std::chrono::system_clock::now();
				timeLock.lock();
				clientPtr->setActiveTime(curTime);
				timeLock.unlock();
				auto t = std::chrono::system_clock::to_time_t(curTime);
				stdLock.lock();
				std::cout << "ClientId = " << hbMsg.clientId << " recvMsg:set active time:" << std::put_time(std::localtime(&t), "%Y-%m-%d %X") << std::endl;
				stdLock.unlock();
				break;
			}
			case CHATMSG:
			{
				ChatMsg msg;
				deserialize(&msg, str, length);
				char buff[1024] = "";
				produceChatMsg(buff, msg.content, msg.clientId, length);
				vLock.lock();
				for (auto c : clientList)
				{
					if (c->getId() != msg.clientId)
					{
						send(c->getSocket(), buff, length, 0);
					}
					
				}
				vLock.unlock();
				break;
			}
			default:
				break;
		}
	}
}

void processNewClient(const SOCKET& soc)
{
	curClientId++;
	auto p = new Client(soc, std::chrono::system_clock::now(), curClientId);
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

	//start heart test
	std::thread heartT(heartBeat);
	heartT.detach();

	int len;
	len = sizeof(SOCKADDR_IN);
	SOCKET newConnection;
	//connection
	SOCKADDR_IN clientaddr;
	while (true)
	{
		newConnection = accept(listenSocket, (SOCKADDR *)&clientaddr, &len);
		if (!validationAccept("accept failed with error:", newConnection))
		{
			continue;
		}
		processNewClient(newConnection);
	}
    WSACleanup(); //clean up Ws2_32.dll to do :need to add validation
	return 0;
}