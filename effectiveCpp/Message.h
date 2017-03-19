#include <cstring>
#include <string>


const unsigned int HEARTBEAT = 1;
const unsigned int CHATMSG = 2;

struct Msg
{
	unsigned int type;
	unsigned int clientId;
};

struct ChatMsg : Msg
{
	unsigned int contentLength;
	char content[200];
	

};


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

//to do 组包函数
void producePacket(const char* content, unsigned int length, char* buffer)
{
	//@HEAD + 长度变量所占长度（１－２５５） +　长度字节数组　＋　内容
	//eg. @HEAD(1-2^32占4个byte)ABCDE
	memcpy(buffer, "@HEAD", 5);
	memcpy(buffer + 5, &length, sizeof(unsigned int));
	memcpy(buffer + 5 + sizeof(unsigned int), content, length);
}

void produceChatMsg(char* buffer, const std::string& content, unsigned int id)
{
	ChatMsg msg;
	msg.clientId = id;
	msg.type = CHATMSG;
	msg.contentLength = content.length() + 1;//这里用string是因为content是作为字符串含义而不是字节数组
	strcpy(msg.content, content.c_str());
	memcpy(buffer, &msg, sizeof(msg));
}

//拆包函数msgBuff:未处理的字节流，buff：当前接受到的字节流，str：内容字符串
bool splitPacket(const char* curBuff, char* str, std::string& msgBuff, unsigned int& length)
{
	msgBuff.append(curBuff, 1024);
	auto pos = msgBuff.find("@HEAD");
	if (pos != std::string::npos)
	{
		std::size_t testPos = pos + 4 + 4;
		//消息长度的长度位是否越界
		if (!isOverStringBound(testPos, msgBuff))
		{
			unsigned int msgLengthLength = msgBuff[pos + 5];
			testPos += msgLengthLength;
			//消息长度位是否越界
			if (!isOverStringBound(testPos, msgBuff))
			{
				std::string lengthStr = msgBuff.substr(pos + 5 + 1, msgLengthLength);
				unsigned int msgLength = std::atoi(lengthStr.c_str());
				testPos += msgLength;
				//消息是否越界
				if (!isOverStringBound(testPos, msgBuff))
				{
					length = msgLength;
					msgBuff.copy(str, msgLength, testPos - msgLength + 1);
					msgBuff.erase(0, testPos + 1);
					return true;
				}

			}
		}

	}
	return false;
}