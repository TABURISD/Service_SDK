// service_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "service_sdk.h"

string user = "444444";
string pwd = "444444";

int _tmain(int argc, _TCHAR* argv[])
{
	int temp;
	string temp1,temp2;

	temp = test();
	//temp1 = login("444444", "44444", pwd);
	//temp1 = login("444444", "444443", pwd);
	//temp1 = login("444444", "444444", pwd);
	//temp1 = login("444444", "444444", pwd);
	temp2 = getRoadList();
	temp2 = getDeviceList();

	return 0;
}

