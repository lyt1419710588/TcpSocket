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
set cmd=%cmd% nClient=1
EasyTCPServer %cmd% 

::��ͣ
pause