// service_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "service_sdk.h"

string user = "444444";
string pwd = "444444";
string deviceid = "W001_85X1-8986031746202300950G";

int _tmain(int argc, _TCHAR* argv[])
{
	string temp1,temp2;
	vector <Roadlist> temp3;
	vector <DeviceList> temp4;
	vector <UserList> temp5;
	vector <MusicList> temp6;
	SystemState temp7;
	int8_t timing[12] = { 0,0,0,0,0,0,0,0,0,0,0,1 };
	int8_t immediately[2] = { 0,0 };

	//temp1 = login("444444", "44444", pwd);
	//temp1 = login("444444", "444443", pwd);
	//temp1 = login("444444", "444444", pwd);
	temp1 = login(user, pwd, pwd);
	temp2 = getRoadList();
	temp3 = handleRoadList(temp2);
	temp2 = getDeviceList();
	temp4 = handleDeviceList(temp2);
	temp2 = getUserList();
	temp5 = handleUserList(temp2);

	//temp2 = addUser("199563", "199563", "199563", "4", "103");
	//temp2 = addUser("120830", "120830", "120830", "4", "107");
	

	string tempstr;

	tempstr = mqttAskList(user, pwd, deviceid);
	temp6 = handleMusicList(tempstr);
	tempstr = mqttSetTTS(user, pwd, deviceid, "18182424");
	tempstr = mqttSetPTT(user, pwd, deviceid);
	tempstr = mqttSetSYS(user, pwd, deviceid, timing, immediately, Quickly, Continuity, temp6[0]);
	tempstr = mqttSetSYS(user, pwd, deviceid, timing, immediately, Quickly, Continuity, temp6[11]);
	tempstr = mqttAskSystem(user, pwd, deviceid, &temp7);

	return 0;
}

