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

//�������msgBuff:δ������ֽ�����buff����ǰ���ܵ����ֽ�����str�������ַ���
bool splitPacket(const char* buff, std::string& str, std::string& msgBuff)
{
	msgBuff += buff;
	auto pos = msgBuff.find("@HEAD");
	if (pos != std::string::npos)
	{
		std::size_t testPos = pos + 5;
		//��Ϣ���ȵĳ���λ�Ƿ�Խ��
		if (!isOverStringBound(testPos, msgBuff))
		{
			unsigned int msgLengthLength = msgBuff[pos + 5];
			testPos += msgLengthLength;
			//��Ϣ����λ�Ƿ�Խ��
			if (!isOverStringBound(testPos, msgBuff))
			{
				std::string lengthStr = msgBuff.substr(pos + 5 + 1, msgLengthLength);
				unsigned int msgLength = std::atoi(lengthStr.c_str());
				testPos += msgLength;
				//��Ϣ�Ƿ�Խ��
				if (!isOverStringBound(testPos, msgBuff))
				{
					str = msgBuff.substr(testPos - msgLength + 1, msgLength);
					msgBuff.erase(0, testPos + 1);
				}
				return true;
			}
		}

	}
	return false;
}



//to do �������
std::string producePacket(std::string& msg)
{
	//@HEAD + ���ȱ�����ռ���ȣ������������� +�������ֽ����顡��������
	//eg. @HEAD15ABCDE
	std::string lengthStr = std::to_string(msg.length());
	unsigned int length = (unsigned int)lengthStr.length();
	std::string packet = "@HEAD";
	packet += (char)length;
	packet += lengthStr;
	packet += msg;
	return packet;
}

