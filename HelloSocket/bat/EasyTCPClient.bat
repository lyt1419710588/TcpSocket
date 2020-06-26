@echo off

::服务端IP地址
set cmd="strIP=127.0.0.1"
::端口
set cmd=%cmd% nPort=4567
::消息处理线程数量
set cmd=%cmd% nThread=1
::客户端数量上限，暂未使用
set cmd=%cmd% nClient=1
::数据会先写入缓冲区
::等待socket可写时才发送
::每个客户端在nSendSleep毫秒时间内
::最大可写入100字节nMsg条数据
set cmd=%cmd% nMsg=100
set cmd=%cmd% nSendSleep=1000
::客户端发送缓冲区大小
set cmd=%cmd% nSendBuffSize=81920
::客户端接收缓冲区大小
set cmd=%cmd% nRecvBuffSize=81920
::检查接收到的服务端消息ID是否连续
set cmd=%cmd% -checkMsgID
EasyTCPClient %cmd% 

::暂停
pause