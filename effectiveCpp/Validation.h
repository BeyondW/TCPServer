#include <iostream>
#include <WinSock2.h> 
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
