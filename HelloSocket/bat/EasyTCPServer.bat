::服务端IP地址
@set strIP=any
::端口
@set nPort=4567
::消息处理线程数量
@set nThread=1
::客户端数量上限，暂未使用
@set nClient=3

EasyTCPServer %strIP% %nPort% %nThread% %nClient%

::暂停
@pause