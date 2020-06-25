#include <stdlib.h>
#include "Alloctor.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include "CELLTimestamp.hpp"
#include "CELLIObjectPool.h"
using namespace std;


class ClassA :public ObjectPoolBase<ClassA, 5>
{
public:
	ClassA(int n)
	{
		num = n;
		printf("ClassA\n");
	}
	~ClassA()
	{
		printf("~ClassA\n");
	}
private:
	int num = 0;

};

const int tCount = 4;
const int mCount = 8;
const int nCount = mCount / tCount;
void workfun(int index)
{
	//m.lock();//临界区开始
	//lock_guard<mutex> lg(m); //自旋锁
	ClassA* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = ClassA::createObj(6);
	}

	for (size_t i = 0; i < nCount; i++)
	{
		ClassA::deleteObj(data[i]);
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

	/*std::shared_ptr<int> b = std::make_shared<int>();
	*b = 100;
	getchar();*/

	/*ClassA *a1 = ClassA::createObj();
	ClassA::deleteObj(a1);*/
	ClassA *b = new ClassA(5);
	printf("---------1-----------\n");
	{
		std::shared_ptr<ClassA> a1 = std::make_shared<ClassA>(5);
	}
	printf("---------2-----------\n");
	{
		std::shared_ptr<ClassA> a2(new ClassA(5));
	}
	printf("---------3-----------\n");
	ClassA *a3 = new ClassA(5);
	delete a3;
	printf("---------4-----------\n");
	getchar();
	return 0;

}