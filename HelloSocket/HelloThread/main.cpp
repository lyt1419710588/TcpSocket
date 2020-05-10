#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "CELLTimestamp.hpp"
using namespace std;

mutex m;
const int tCount = 4;
atomic_int sum = 0;
void workfun(int index)
{
	//m.lock();//�ٽ�����ʼ
	//lock_guard<mutex> lg(m); //������
	for (int  i = 0; i < 20000000; i++)
	{
		
		sum++;
		
	}//�̰߳�ȫ���̲߳���ȫ
	//m.unlock();//�ٽ�������
	//cout << index << "hello,other thread" << endl;
}
int main()
{
	thread *t[tCount];
	for (int  i = 0; i < tCount; i++)
	{
		t[i] = new thread(workfun, i);
	}
	CELLTimestamp ctime;
	for (int i = 0; i < tCount; i++)
	{
		t[i]->join();
	}
	//t.join();
	cout << "cTime = " << (long long)ctime.getElaspedInMillSec()<< "sum = " << sum << endl;

	ctime.update();
	sum = 0;
	for (int i = 0; i < 80000000; i++)
	{
		//m.lock();//�ٽ�����ʼ
		sum++;
		//m.unlock();//�ٽ�������
	}
	cout << "cTime = " << (long long)ctime.getElaspedInMillSec() << "sum = " << sum << endl;
	cout << "hello,main thread" << endl;
	getchar();
	return 0;
	
}