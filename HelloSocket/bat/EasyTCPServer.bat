@echo off
::服务端IP地址
::set strIP=any
::端口
::set nPort=4567
::消息处理线程数量
::set nThread=1
::客户端数量上限，暂未使用
::set  nClient=1
::key-val

::服务端IP地址
set cmd="strIP=127.0.0.1"
::端口
set cmd=%cmd% nPort=4567
::消息处理线程数量
set cmd=%cmd% nThread=1
::客户端数量上限，暂未使用
set cmd=%cmd% nMaxClient=10240
::客户端发送缓冲区大小
set cmd=%cmd% nSendBuffSize=81920
::客户端发送缓冲区大小
set cmd=%cmd% nRecvBuffSize=81920
::接收消息后将返回消息应答
set cmd=%cmd% -sendback
::提示发送缓冲区已满
::当出现sendfull时，表示当次消息被抛弃
set cmd=%cmd% -sendfull
::检查接收客户端的消息id是否连续
set cmd=%cmd% -checkMsgID
::自定义标志，未使用
set cmd=%cmd% -p
EasyTCPServer %cmd% 

::暂停
pause