@echo off
::�����IP��ַ
::set strIP=any
::�˿�
::set nPort=4567
::��Ϣ�����߳�����
::set nThread=1
::�ͻ����������ޣ���δʹ��
::set  nClient=1
::key-val

::�����IP��ַ
set cmd="strIP=127.0.0.1"
::�˿�
set cmd=%cmd% nPort=4567
::��Ϣ�����߳�����
set cmd=%cmd% nThread=1
::�ͻ����������ޣ���δʹ��
set cmd=%cmd% nMaxClient=10240
::�ͻ��˷��ͻ�������С
set cmd=%cmd% nSendBuffSize=81920
::�ͻ��˷��ͻ�������С
set cmd=%cmd% nRecvBuffSize=81920
::������Ϣ�󽫷�����ϢӦ��
set cmd=%cmd% -sendback
::��ʾ���ͻ���������
::������sendfullʱ����ʾ������Ϣ������
set cmd=%cmd% -sendfull
::�����տͻ��˵���Ϣid�Ƿ�����
set cmd=%cmd% -checkMsgID
::�Զ����־��δʹ��
set cmd=%cmd% -p
EasyTCPServer %cmd% 

::��ͣ
pause