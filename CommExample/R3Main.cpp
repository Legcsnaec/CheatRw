#include <iostream>
#include "Comm.h"
using namespace std;

int main()
{
	int ret = -1;
	ULONG data = 0x123456;
	ret = CommWin10(DriverRead, &data, 4);
	printf("%d\n", ret);

	system("pause");
	return 0;
}