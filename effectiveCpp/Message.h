#include <cstring>
#include <string>


const unsigned int HEARTBEAT = 1;

struct Msg
{
	unsigned int length;
	unsigned int type;
	unsigned int clientId;
};

struct ChatMsg : Msg
{
	char* userName;
	char* content;
};


void serializeChatMsg(char* dst, const void* src, unsigned int length)
{
	memcpy(dst, src, length);
}

void deserializeChatMsg(void* dst, const char* src)
{
	//get msg length
	unsigned int length;
	memcpy(&length, src, sizeof(unsigned int));
	memcpy(dst, src, length);
}

void deserialize(void* dst, const char* src, unsigned int length)
{
	memcpy(dst, src, length);
}

bool isOverStringBound(std::size_t pos, const std::string& str)
{
	return pos > str.length() ? true : false;

}

unsigned int getType(const char* src)
{
	unsigned int i = 0;
	memcpy(&i, src, sizeof(unsigned int));
	return i;
}

//拆包函数
bool splitPacket(const char* buff, std::string& str, std::string& msgBuff)
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