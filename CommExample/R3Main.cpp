#include <iostream>
#include "Comm.h"
using namespace std;

int main()
{
	PPACKET packet = (PPACKET)malloc(sizeof(PACKET));
	memset(packet, 0, sizeof(PACKET));
	packet->commFlag = IsR3ToR0;
	packet->commFnID = DriverRead;

	int ret = CommWin7(packet);

	printf("%d\n", (packet->result));

	free(packet);
	system("pause");
	return 0;
}