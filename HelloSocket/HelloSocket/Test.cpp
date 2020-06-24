#include "EasyTcpClient.hpp"

#include "CELLStream.hpp"
#include "CELLMSGStream.hpp"
class MyClient :public EasyTcpClient
{
public:
	void OnNetMsg(DataHeader* header)
	{
		//����ͻ�������
		switch (header->cmd)
		{
		case CMD_LOGOUT_RESULT:
		{
			//���շ��������ص�����
			/*LoginResult *ret = (LoginResult*)header;*/
			// CELLLog_Info("�յ��������Ϣ��retLogin = %d�����ݳ���:%d\n", ret->result, header->dataLength);

			CELLRecvStream r(header);
			auto s1 = r.readInt8();
			auto s2 = r.readInt16();
			auto s3 = r.readInt32();
			auto s4 = r.readFloat();
			auto s5 = r.readDouble();


			char strName1[10] = {};
			r.ReadArray(strName1, 10);

			int pas1[10] = {};
			r.ReadArray(pas1, 10);

		}
		break;
		case CMD_ERROR:
		{
			//���շ��������ص�����
			CELLLog_Info("�յ��������Ϣ��CMD_ERROR  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		default:
		{
			CELLLog_Info("�յ������δ�������ݣ�  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		}
	}
};
int  main()
{
	CELLSendStream s;

	s.setNetCmd(CMD_LOGOUT);
	s.writeInt8(15);
	s.writeInt16(15);
	s.writeInt32(15);
	s.writeFloat(15.333f);
	s.writeDouble(15.333f);
	char* strName = "client";
	s.writeArray(strName,strlen(strName));

	int pas[5] = {1,2,3,4,5};
	s.writeArray(pas, 5);
	s.finish();
	/*auto sLength = s.readInt16();
	auto scmd = s.readInt16();
	auto s1 = s.readInt8();
	auto s2 = s.readInt16();
	auto s3 = s.readInt32();
	auto s4 = s.readFloat();
	auto s5 = s.readDouble();


	char strName1[10] = {};
	s.ReadArray(strName1, 10);

	int pas1[10] = {};
	s.ReadArray(pas1, 10);*/


	MyClient client;
	client.Connect("127.0.0.1", 4568);
	client.SendData(s.Data(),s.length());
	while (client.isRun())
	{
		client.OnRun();
		CELLThread::Sleep(10);
	}
	return 0;
}