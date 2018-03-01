#ifndef SERVICE_SDK_H_
#define SERVICE_SDK_H_

#include "md5.h"
#include <string>
#include <vector>
#include "json/json.h"
#include "afxinet.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <sstream>
#include "mqtt/async_client.h"

using namespace std;
using std::string;
using std::vector;

struct Roadlist {
	int id;				//道路的id号
	string name;		//道路的名字（如果是无人机预警器该属性表示单位名称）
	string endpoint1_id;	//起点id
	string endpoint2_id;	//终点id
	string endpoint1;	//起点
	string endpoint2;	//终点
};

struct DeviceList {
	string ID;			//设备ID
	string Model;		//设备类型
	string Phone;		//设备绑定的手机号码
	int Road;			//设备所在的道路的id（如果是无人机预警设备对应的就是单位id）
	string RoadName;	//设备所在的道路名称（如果是无人机预警设备对应的就是单位名称）
	int Direct;			//（所在道路的方向，0表示正方向，1表示反方向）
	int Mile;			//在Road表示的道路 Direct 所表示的方向的多少米位置
	string PPTGroupName;//对讲设备所在组
	string PPTDeviceName;//对讲设备名称
};

struct UserList {
	string Username;	//用户名
	string Nick;		//用户昵称
	int Role;			//用户角色（对应着用户权限）
};

struct MusicList {
	int id;				//曲目id号
	wstring name;		//曲目的名字
};

enum LightType { NoLight, Quickly, Slowly };

enum ModeType { Immediately, Continuity, Timing, Stop, Trigger };

struct SystemState {
	int8_t timing[12];		//当前设置时间段
	int8_t immediately[2];	//当前设置插播时长
	LightType lighttype;	//当前设置警灯状态
	ModeType modetype;	//当前设置功能
	int8_t music;			//当前设置播放曲目编号
	int8_t time[2];			//上次系统设置时间
};

extern string login(string username, string password, string myControllerId);
extern string logout(void);
extern string getRoadList(void);
extern string getDeviceList(void);
extern string getUserList(void);
extern string addUser(string username, string password, string nick, string role);
extern string deleteUser(string username);
extern string mqttAskSystem(string username, string password, string deviceid, SystemState* systemstate);
extern string mqttAskList(string username, string password, string deviceid);
extern string mqttSetTTS(string username, string password, string deviceid, string gbk);
extern string mqttSetPTT(string username, string password, string deviceid);
//extern string mqttSetSYS(string username, string password, string deviceid, int8_t *timing, int8_t *immediately, LightType lighttype, ModeType modetype, MusicList music);
extern string mqttSetDeviceTiming(string username, string password, string deviceid, int8_t *timing, LightType lighttype, MusicList music);
extern string mqttSetDeviceStop(string username, string password, string deviceid, LightType lighttype, MusicList music);
extern string mqttSetDeviceTrigger(string username, string password, string deviceid, LightType lighttype, MusicList music);
extern string mqttSetDeviceContinuity(string username, string password, string deviceid, LightType lighttype, MusicList music);
extern string mqttSetDeviceImmediately(string username, string password, string deviceid, int8_t *immediately, LightType lighttype, MusicList music);
extern vector <Roadlist> handleRoadList(string RoadListInfo);
extern vector <DeviceList> handleDeviceList(string DeviceListInfo);
extern vector <UserList> handleUserList(string UserListInfo);
extern vector <MusicList> handleMusicList(string MusicListInfo);

#endif