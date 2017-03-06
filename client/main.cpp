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
			cout << "����������: " << buf << endl;
		}
		cout << "�������Է���������Ϣ: " << buf << endl;
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
	WSAStartup(MAKEWORD(1, 1), &wsa);	//��ʼ��WS2_32.DLL

	SOCKET clientSocket;
	//�����׽���
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9102);
	serveraddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	connect(clientSocket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	cout << "���ӳɹ�!" << endl;

	//��������
	cout << "�����뷢�͸����������ַ�:" << endl;

	rsnonBlockExample(clientSocket);

	//��������
	
	//�ر��׽���
	closesocket(clientSocket);
	WSACleanup();    //�ͷ�WS2_32.DLL
	return 0;
}