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
	//m.lock();//临界区开始
	//lock_guard<mutex> lg(m); //自旋锁
	for (int  i = 0; i < 20000000; i++)
	{
		
		sum++;
		
	}//线程安全，线程不安全
	//m.unlock();//临界区结束
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
		//m.lock();//临界区开始
		sum++;
		//m.unlock();//临界区结束
	}
	cout << "cTime = " << (long long)ctime.getElaspedInMillSec() << "sum = " << sum << endl;
	cout << "hello,main thread" << endl;
	getchar();
	return 0;
	
}