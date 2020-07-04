@echo off

#服务端IP地址
cmd="strIP=127.0.0.1"
#端口
cmd=$cmd' nPort=4567'
#消息处理线程数量
cmd=$cmd' nThread=1'
#客户端数量上限，暂未使用
cmd=$cmd' nClient=1'
#数据会先写入缓冲区
#等待socket可写时才发送
#每个客户端在nSendSleep毫秒时间内
#最大可写入100字节nMsg条数据
cmd=$cmd' nMsg=100'
cmd=$cmd' nSendSleep=1000'
#客户端发送缓冲区大小
cmd=$cmd' nSendBuffSize=81920'
#客户端接收缓冲区大小
cmd=$cmd' nRecvBuffSize=81920'
#检查接收到的服务端消息ID是否连续
cmd=$cmd' -checkMsgID'
./client $cmd 

#暂停
read -p "请按任意键......" var