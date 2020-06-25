#include <functional>
//function
//lambda

int funA(int a, int b)
{
	printf("a + b = %d\n", a + b);
	return a + b;
}

int main()
{
	/*std::function<int(int, int)> call = funA;
	int c = call(10, 5);*/

	std::function<int(int, int)> call;

	//lambda表达式
	int n = 5;
	call = [n](int a, int b) -> int {
		return 10;
	};
	n = call(5, 6);
	return 0;
}