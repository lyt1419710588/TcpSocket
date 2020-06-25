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
set cmd=%cmd% nClient=1
EasyTCPServer %cmd% 

::暂停
pause