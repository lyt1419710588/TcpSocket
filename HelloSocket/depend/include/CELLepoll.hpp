#ifndef _CELLEPOLL_HPP
#define _CELLEPOLL_HPP


#if __linux__

#include "Cell.hpp"
#include "CellClient.hpp"
#include "CELLLog.hpp"
#include <sys/epoll.h>

#define EPOLL_ERROR     (-1)
class CEllEpoll
{
public:
    ~CEllEpoll()
    {
        eclose();
    }
    int create(int maxEvents)
    {
         //linux 2.6.8之后 size就没有作用了
        //由epoll 动态管理通过cat /proc/sys/fs/file-max 获取
        if(_epfd > 0)
        {
            return _epfd;
        }
        _maxEvents = maxEvents;
        _epfd =  epoll_create(_maxEvents);
        if(EPOLL_ERROR == _epfd)
        {
            perror("epoll_create error\n");
        }
        _pEvent = new epoll_event[_maxEvents];
        return _epfd;
    }

    int ctrl(int op,SOCKET sockfd,uint32_t events)
    {
        epoll_event ev;
        //事件类型，关心可读事件EPOLL_ERROR
        ev.events = events;
        //事件关联socket描述符
        ev.data.fd = sockfd;
        //像epoll 对象注册需要管理，监听的socket文件描述符
        //并且说明关心的事件
        //返回 0代表成功，返回负值代表失败，一般返回-1
        int ret = epoll_ctl(_epfd,op,sockfd,&ev);
        if(EPOLL_ERROR == ret)
        {
            CELLLog_Error("epoll_ctrl error");
        }
        return ret;
    }

    int ctrl(int op,std::shared_ptr<CellClient> pClient,uint32_t events)
    {
        epoll_event ev;
        //事件类型，关心可读事件EPOLL_ERROR
        ev.events = events;
        //事件关联socket描述符
        ev.data.ptr = pClient.get();
        //像epoll 对象注册需要管理，监听的socket文件描述符
        //并且说明关心的事件
        //返回 0代表成功，返回负值代表失败，一般返回-1
        int ret = epoll_ctl(_epfd,op,pClient->getSocket(),&ev);
        if(EPOLL_ERROR == ret)
        {
            CELLLog_Error("epoll_ctrl error");
        }
        return ret;
    }

    int wait(int timeout)
    {
        int ret = epoll_wait(_epfd,_pEvent,_maxEvents,timeout);
        if (EPOLL_ERROR ==  ret)
        {
            CELLLog_Error("epoll_wait error");
        }
        return ret;
    }

    epoll_event* event()
    {
        return _pEvent;
    }

     
    //关闭epoll描述符
    void eclose()
    {
        if(_pEvent)
        {
            delete[] _pEvent;
            _pEvent = nullptr;
        }
        if(_epfd > 0)
        {
            close(_epfd);
            _epfd = EPOLL_ERROR;
        }
    }
   
private:
    epoll_event *_pEvent = nullptr;
    int _epfd = EPOLL_ERROR;
    int _maxEvents = 0;
    
};

#endif // __linux__
#endif //_CELLEPOLL_HPP