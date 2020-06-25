#服务端口ip
#strIP=any
#服务端短裤
#nPort=4567
#服务线程数量
#nThread=1
#服务承载最大客户端数量
#nClient=3
#key-val

#服务端IP地址
cmd="strIP=127.0.0.1"
#端口
cmd=$cmd' nPort=4567'
#消息处理线程数量
cmd=$cmd' nThread=2'
#客户端数量上限，暂未使用
cmd=$cmd' nClient=3'
./server $cmd

read -p "请按任意键......" var
