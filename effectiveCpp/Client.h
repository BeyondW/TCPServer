#include <chrono>
#include <mutex>
#include <WinSock2.h> 
typedef std::chrono::system_clock::time_point TimePoint;
class Client
{
public:
	Client(SOCKET s, TimePoint curTime, unsigned int id) :soct(s), activeTime(curTime), clientId(id){};
	~Client();
	const TimePoint& getActiveTime() const;
	void setActiveTime(const TimePoint& curTime);
	const SOCKET& getSocket() const;
	void setId(unsigned int id);
private:

	TimePoint activeTime;
	SOCKET soct;
	unsigned int clientId;
};