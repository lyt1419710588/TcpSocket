#服务端口ip
strIP=any
#服务端短裤
nPort=4567
#服务线程数量
nThread=1
#服务承载最大客户端数量
nClient=3

./server $strIP $nPort $nThread $nClient

read -p "请按任意键......"
