#ifndef SERVICE_SDK_H_
#define SERVICE_SDK_H_

#include "md5.h"
#include <string>
#include "json/json.h"
#include "afxinet.h"

using namespace std;
using std::string;

extern int test(void);
extern int test1(void);
extern string login(string user, string pwd, string myControllerId);
extern string getRoadList(void);
extern string getDeviceList(void);
extern string getUserList(void);
extern string addUser(string userName, string pwd, string nick, string group, string role);
extern string deleteUser(string userName);

#endif