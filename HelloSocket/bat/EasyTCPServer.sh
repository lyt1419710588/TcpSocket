#服务端口ip
#strIP=any
#服务端短裤
#nPort=4567
#服务线程数量
#nThread=1
#服务承载最大客户端数量
#nClient=3
#key-val
#突破linuxsocket 1024限制
nOpenFile=`ulimit -n`
if [ $nOpenFile -lt 10240];then
	echo "重置当前进程可以打开的最大文件数"
	unlimit -n 10240
fi
echo "当前进程可以打开的最大文件数"
unlimit -n
#服务端IP地址
cmd="strIP=127.0.0.1"
#端口
cmd=$cmd' nPort=4567'
#消息处理线程数量
cmd=$cmd' nThread=2'
#客户端数量上限，暂未使用
cmd=$cmd' nClient=3'

#客户端数量上限，暂未使用
cmd=$cmd' nMaxClient=10240'
#客户端发送缓冲区大小
cmd=$cmd' nSendBuffSize=81920'
#客户端发送缓冲区大小
cmd=$cmd' nRecvBuffSize=81920'
#接收消息后将返回消息应答
cmd=$cmd' -sendback'
#提示发送缓冲区已满
#当出现sendfull时，表示当次消息被抛弃
cmd=$cmd' -sendfull'
#检查接收客户端的消息id是否连续
cmd=$cmd' -checkMsgID'
#自定义标志，未使用
cmd=$cmd' -p'
./server $cmd

read -p "请按任意键......" var
