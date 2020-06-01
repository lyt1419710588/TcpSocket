#include <stdlib.h>
#include "Alloctor.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include "CELLTimestamp.hpp"
using namespace std;

const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;
void workfun(int index)
{
	//m.lock();//临界区开始
	//lock_guard<mutex> lg(m); //自旋锁
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[rand() % 1024 + 1];
	}

	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
	 //m.unlock();//临界区结束
	 //cout << index << "hello,other thread" << endl;
}
int main()
{
	//thread *t[tCount];
	//for (int i = 0; i < tCount; i++)
	//{
	//	t[i] = new thread(workfun, i);
	//}
	//CELLTimestamp ctime;
	//for (int i = 0; i < tCount; i++)
	//{
	//	t[i]->join();
	//}
	////t.join();

	//cout << "cTime = " << (long long)ctime.getElaspedInMillSec() <<  endl;
	//cout << "hello,main thread" << endl;

	std::shared_ptr<int> b = std::make_shared<int>();
	*b = 100;
	getchar();
	return 0;

}