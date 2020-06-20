#include "EasyTcpClient.hpp"

#include "CELLStream.hpp"

class MyClient :public EasyTcpClient
{
public:
	void OnNetMsg(DataHeader* header)
	{
		//����ͻ�������
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//���շ��������ص�����
			LoginResult *ret = (LoginResult*)header;
			// CELLLog::Info("�յ��������Ϣ��retLogin = %d�����ݳ���:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			//���շ��������ص�����
			CELLLog::Info("�յ��������Ϣ��CMD_ERROR  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		default:
		{
			CELLLog::Info("�յ������δ�������ݣ�  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		}
	}
};
int  main()
{
	CELLStream s;

	s.writeInt8(15);
	s.writeInt16(15);
	s.writeInt32(15);
	s.writeFloat(15.333f);
	s.writeDouble(15.333f);
	char* strName = "laoli";
	s.writeArray(strName,strlen(strName));

	int pas[5] = {1,2,3,4,5};
	s.writeArray(pas, 5);

	auto s1 = s.readInt8();
	auto s2 = s.readInt16();
	auto s3 = s.readInt32();
	auto s4 = s.readFloat();
	auto s5 = s.readDouble();


	char strName1[10] = {};
	s.ReadArray(strName1, 10);

	int pas1[10] = {};
	s.ReadArray(pas1, 10);


	MyClient client;
	client.Connect("127.0.0.1", 4567);
	while (client.isRun())
	{
		client.OnRun();
		CELLThread::Sleep(500);
	}
	return 0;
}