#ifndef _INETEVEMT_HPP_
#define _INETEVEMT_HPP_

#include <memory>
#include "CellClient.hpp"
//�����¼��ӿ�
class CellServer;
class INetEvent
{
public:
	//���麯��
	//�ͻ��˼���ʱ֪ͨ���ͻ����뿪�¼�
	virtual void OnNetJoin(std::shared_ptr<CellClient> pClient) = 0;
	//�ͻ����뿪ʱ֪ͨ���ͻ����뿪�¼�
	virtual void OnNetLeave(std::shared_ptr<CellClient> pClient) = 0;
	//�ͻ��˶��յ���Ϣ��֪ͨ���߳�
	virtual void OnNetMsg(CellServer* pCellServer, std::shared_ptr<CellClient> pClient, DataHeader*  header) = 0;
	//recv�¼�
	virtual void OnNetRecv(std::shared_ptr<CellClient> pClient) = 0;
private:

};


#endif // !_INETEVEMT_HPP_

