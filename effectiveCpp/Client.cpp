#include "Client.h"
const TimePoint& Client::getActiveTime() const
{
	return activeTime;
}

void Client::setActiveTime(const TimePoint& curTime)
{

	activeTime = curTime;

}

const SOCKET& Client::getSocket() const
{
	return soct;
}

Client::~Client()
{
	int code = closesocket(soct);
	if (code == SOCKET_ERROR)
	{
		//log
	}
}

void Client::setId(unsigned int id)
{
	clientId = id;
}

unsigned int Client::getId()
{
	return clientId;
}

