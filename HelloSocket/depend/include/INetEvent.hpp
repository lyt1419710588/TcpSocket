#ifndef _INETEVEMT_HPP_
#define _INETEVEMT_HPP_

#include <memory>
#include "CellClient.hpp"
//网络事件接口
class CellServer;
class INetEvent
{
public:
	//纯虚函数
	//客户端加入时通知，客户端离开事件
	virtual void OnNetJoin(std::shared_ptr<CellClient> pClient) = 0;
	//客户端离开时通知，客户端离开事件
	virtual void OnNetLeave(std::shared_ptr<CellClient> pClient) = 0;
	//客户端端收到消息后通知主线程
	virtual void OnNetMsg(CellServer* pCellServer, std::shared_ptr<CellClient> pClient, DataHeader*  header) = 0;
	//recv事件
	virtual void OnNetRecv(std::shared_ptr<CellClient> pClient) = 0;
private:

};


#endif // !_INETEVEMT_HPP_

