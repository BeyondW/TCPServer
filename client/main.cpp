#include <iostream>
#include <thread>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;



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
		if (!memcmp(buf, "h", 1))
		{
			char hBuf[2] = "h";
			send(clientSocket, hBuf, strlen(hBuf) + 1, 0);
			cout << "发送心跳包: " << buf << endl;
		}
		cout << "接收来自服务器的信息: " << buf << endl;
	}
}

void rsnonBlockExample(const SOCKET& clientSocket)
{
	thread t(getMsg, clientSocket);
	t.detach();
	char buf[1024];
	int iResult;
	while (1)
	{
		cin >> buf;
		iResult = send(clientSocket, buf, strlen(buf) + 1, 0);
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