#include <chrono>
#include <mutex>
#include <WinSock2.h> 
typedef std::chrono::system_clock::time_point TimePoint;
class Client
{
public:
	Client(SOCKET s, TimePoint curTime) :soct(s), activeTime(curTime){};
	const TimePoint& getActiveTime() const;
	void setActiveTime(const TimePoint& curTime);
	const SOCKET& getSocket() const;
private:

	TimePoint activeTime;
	SOCKET soct;
};