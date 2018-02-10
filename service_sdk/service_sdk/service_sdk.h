#ifndef SERVICE_SDK_H_
#define SERVICE_SDK_H_

#include "md5.h"
#include <string>
#include "json/json.h"
#include "afxinet.h"

using std::string;

extern int test(void);
extern int test1(void);
extern string login(string user, string pwd, string myControllerId);
extern string getRoadList(void);

#endif