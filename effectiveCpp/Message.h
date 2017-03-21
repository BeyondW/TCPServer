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

//to do �������
void producePacket(const char* content, unsigned int length, char* buffer)
{
	//@HEAD + ���ȱ�����ռ���ȣ������������� +�������ֽ����顡��������
	//eg. @HEAD(1-2^32ռ4��byte)ABCDE
	memcpy(buffer, "@HEAD", 5);
	memcpy(buffer + 5, &length, sizeof(unsigned int));
	memcpy(buffer + 5 + sizeof(unsigned int), content, length);
}

void produceChatMsg(char* buffer, const std::string& content, unsigned int id)
{
	ChatMsg msg;
	msg.clientId = id;
	msg.type = CHATMSG;
	msg.contentLength = content.length() + 1;//������string����Ϊcontent����Ϊ�ַ�������������ֽ�����
	strcpy(msg.content, content.c_str());
	memcpy(buffer, &msg, sizeof(msg));
}

//�������msgBuff:δ������ֽ�����buff����ǰ���ܵ����ֽ�����str�������ַ���
bool splitPacket(const char* curBuff, unsigned int recvLength, char* str, std::string& msgBuff, unsigned int& length)
{
	msgBuff.append(curBuff, recvLength);
	auto pos = msgBuff.find("@HEAD");
	if (pos != std::string::npos)
	{
		std::size_t testPos = pos + 8;
		//��Ϣ�����Ƿ�Խ��
		if (!isOverStringBound(testPos, msgBuff))
		{
			unsigned int msgLength = 0;
			memcpy(&msgLength, msgBuff.c_str() + testPos - 3, sizeof(unsigned int));
			testPos += msgLength;
			//��Ϣ����λ�Ƿ�Խ��
			if (!isOverStringBound(testPos, msgBuff))
			{
					length = msgLength;
					msgBuff.copy(str, msgLength, testPos - msgLength + 1);
					msgBuff.erase(0, testPos + 1);//erase���ұ�
					return true;
			}
		}
	}
	return false;
}