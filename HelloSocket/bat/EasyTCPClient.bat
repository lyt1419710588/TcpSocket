@echo off

::�����IP��ַ
set cmd="strIP=127.0.0.1"
::�˿�
set cmd=%cmd% nPort=4567
::��Ϣ�����߳�����
set cmd=%cmd% nThread=1
::�ͻ����������ޣ���δʹ��
set cmd=%cmd% nClient=1
::���ݻ���д�뻺����
::�ȴ�socket��дʱ�ŷ���
::ÿ���ͻ�����nSendSleep����ʱ����
::����д��100�ֽ�nMsg������
set cmd=%cmd% nMsg=100
set cmd=%cmd% nSendSleep=1000
::�ͻ��˷��ͻ�������С
set cmd=%cmd% nSendBuffSize=81920
::�ͻ��˽��ջ�������С
set cmd=%cmd% nRecvBuffSize=81920
::�����յ��ķ������ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID
EasyTCPClient %cmd% 

::��ͣ
pause