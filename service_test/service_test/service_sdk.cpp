#include "stdafx.h"
#include "service_sdk.h"

void UTF8toANSI(string &strUTF8);
void ANSItoUTF8(string &strAnsi);
bool isAllDigit(string str);

int user_group;//用户权限组
int user_role;

string str = "{\"uploadid\": \"UP000000\",\"code\": 100,\"msg\": \"\",\"files\": \"\"}???????????????????????????????????????????";

Json::Reader reader;
Json::Value root;

string login(string user,string pwd,string myControllerId)//用户登录
{
	if ((user.length() != 6) || (pwd.length() != 6))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(user)) || (!isAllDigit(pwd)))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号与密码应为6位数字\"}";
	
	MD5 md5;//加密用户密码
	md5.update(pwd);
	string passwordval = md5.toString();
	md5.reset();

	string temp = user;
	string strFormData = "Username=" + temp + "&Password=" + passwordval;

	bool httpresult;

	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/wapi/auth/session"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult=pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[1024];
	string result;
	string getdata = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 1023))>0)
		{
			getdata += szBuff;
		}
	}
	UTF8toANSI(getdata);
	result = getdata;

	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if (root["ErrCode"].asInt() == 0)
	{
		int i = 0;
		user_group = root["Result"]["Groups"][i]["Group"].asInt();
		user_role = root["Result"]["Groups"][i]["UserRole"].asInt();
		return "{ \"ErrCode\":0，\"ErrMsg\":\"登陆成功\"}";
	} else {
		return result;
	}
}

string getRoadList(void)
{
	bool httpresult;

	string strFormData = "";
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, _T("/wapi/gps/roads"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[2048];
	string result;
	string getdata = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 2047))>0)
		{
			getdata += szBuff;
		}
	}
	UTF8toANSI(getdata);
	result = getdata;

	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	return result;
}

string getDeviceList(void)
{
	bool httpresult;

	string temp,strdata;
	strdata = "/wapi/bollard/bollards?Group=" + to_string(user_group);
	string strFormData = "";
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, (CString)strdata.c_str());
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[4096];
	string result;
	string getdata = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 4095))>0)
		{
			getdata += szBuff;
		}
	}
	UTF8toANSI(getdata);
	result = getdata;

	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	return result;
}

string getUserList(void)
{
	bool httpresult;

	string temp, strdata;
	strdata = "/wapi/group/users?Group=" + to_string(user_group);
	string strFormData = "";
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, (CString)strdata.c_str());
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[4096];
	string result;
	string getdata = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 4095))>0)
		{
			getdata += szBuff;
		}
	}
	UTF8toANSI(getdata);
	result = getdata;

	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	return result;
}

string addUser(string userName, string pwd, string usernick, string group, string role)
{
	if ((userName.length() != 6) || (pwd.length() != 6) || (role.length() != 3))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号与密码应为6位数字、权限应为3位数字\"}";
	if ((!isAllDigit(userName)) || (!isAllDigit(pwd)) || (!isAllDigit(group))|| (!isAllDigit(role)))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号、密码、组号、权限应为数字\"}";

	bool httpresult;

	string strFormData = "Username=" + userName + "&Password=" + pwd + "&Nick=" + usernick + "&Group=" + to_string(user_group) + "&Role=" + role;
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/wapi/group/users"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[2048];
	string result;
	string get_temp = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 2047))>0)
		{
			get_temp += szBuff;
		}
	}
	UTF8toANSI(get_temp);
	result = get_temp;
	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	return result;
}

string deleteUser(string userName)
{
	if ((userName.length() != 6))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号应为6位数字\"}";
	if ((!isAllDigit(userName)))
		return "{ \"ErrCode\":20000015，\"ErrMsg\":\"账号应为6位数字\"}";

	bool httpresult;

	string strFormData = "Group=" + to_string(user_group) + "&Username=" + userName;
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_DELETE, _T("/wapi/group/users"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017，\"ErrMsg\":\"服务器连接失败\"}";

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);
	char szBuff[2048];
	string result;
	string get_temp = "";
	if (dwRet == HTTP_STATUS_OK)
	{
		UINT nRead;
		while ((nRead = pFile->Read(szBuff, 2047))>0)
		{
			get_temp += szBuff;
		}
	}
	UTF8toANSI(get_temp);
	result = get_temp;
	pFile->Close();
	delete pFile;
	pFile = NULL;
	pHttpConnect->Close();
	delete pHttpConnect;
	pHttpConnect = NULL;
	session.Close();

	Json::Reader reader;//解析信息
	Json::Value root;

	if (!reader.parse(result, root, false))
	{
		return "{ \"ErrCode\":20000016，\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	return result;
}

list <Roadlist> handleRoadList(string RoadListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	Roadlist temp;

	reader.parse(RoadListInfo, root, false);

	list <Roadlist> templist(0);
	int size = root["Result"].size();

	for (int i = 0; i<size; i++)
	{
		temp.id = root["Result"][i]["ID"].asInt();
		temp.name = root["Result"][i]["Name"].asString();
		temp.endpoint1_id = root["Result"][i]["Endpoint"][0]["ID"].asString();
		temp.endpoint1 = root["Result"][i]["Endpoint"][0]["Name"].asString();
		temp.endpoint2_id = root["Result"][i]["Endpoint"][1]["ID"].asString();
		temp.endpoint2 = root["Result"][i]["Endpoint"][1]["Name"].asString();

		templist.push_back(temp);
	}

	return templist;
}

list <DeviceList> handleDeviceList(string DeviceListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	DeviceList temp;

	reader.parse(DeviceListInfo, root, false);

	list <DeviceList> templist(0);
	int size = root["Result"].size();

	for (int i = 0; i<size; i++)
	{
		temp.ID = root["Result"][i]["ID"].asString();
		temp.Model = root["Result"][i]["Model"].asString();
		temp.Phone = root["Result"][i]["Phone"].asString();
		temp.Road = root["Result"][i]["Road"].asInt();
		temp.RoadName = root["Result"][i]["RoadName"].asString();
		temp.Direct = root["Result"][i]["Direct"].asInt();
		temp.Mile = root["Result"][i]["Mile"].asInt();
		temp.PPTGroupName = root["Result"][i]["PPTGroupName"].asString();
		temp.PPTDeviceName = root["Result"][i]["PPTDeviceName"].asString();

		templist.push_back(temp);
	}

	return templist;
}

list <UserList> handleUserList(string UserListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	UserList temp;

	reader.parse(UserListInfo, root, false);

	list <UserList> templist(0);
	int size = root["Result"].size();

	for (int i = 0; i<size; i++)
	{
		temp.Username = root["Result"][i]["Username"].asString();
		temp.Nick = root["Result"][i]["Nick"].asString();
		temp.Role = root["Result"][i]["Role"].asInt();

		templist.push_back(temp);
	}

	return templist;
}

void UTF8toANSI(string &strUTF8)
{
	//获取转换为多字节后需要的缓冲区大小，创建多字节缓冲区
	UINT nLen = MultiByteToWideChar(CP_UTF8, NULL, strUTF8.c_str(), -1, NULL, NULL);
	WCHAR *wszBuffer = new WCHAR[nLen + 1];
	nLen = MultiByteToWideChar(CP_UTF8, NULL, strUTF8.c_str(), -1, wszBuffer, nLen);
	wszBuffer[nLen] = 0;

	nLen = WideCharToMultiByte(936, NULL, wszBuffer, -1, NULL, NULL, NULL, NULL);
	CHAR *szBuffer = new CHAR[nLen + 1];
	nLen = WideCharToMultiByte(936, NULL, wszBuffer, -1, szBuffer, nLen, NULL, NULL);
	szBuffer[nLen] = 0;

	strUTF8 = szBuffer;
	//清理内存
	delete[]szBuffer;
	szBuffer = NULL;
	delete[]wszBuffer;
	wszBuffer = NULL;
}

void ANSItoUTF8(string &strAnsi)
{
	//获取转换为宽字节后需要的缓冲区大小，创建宽字节缓冲区，936为简体中文GB2312代码页  
	UINT nLen = MultiByteToWideChar(936, NULL, strAnsi.c_str(), -1, NULL, NULL);
	WCHAR *wszBuffer = new WCHAR[nLen + 1];
	nLen = MultiByteToWideChar(936, NULL, strAnsi.c_str(), -1, wszBuffer, nLen);
	wszBuffer[nLen] = 0;
	//获取转为UTF8多字节后需要的缓冲区大小，创建多字节缓冲区  
	nLen = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, NULL, NULL, NULL, NULL);
	CHAR *szBuffer = new CHAR[nLen + 1];
	nLen = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, szBuffer, nLen, NULL, NULL);
	szBuffer[nLen] = 0;

	strAnsi = szBuffer;
	//内存清理  
	delete[]wszBuffer;
	delete[]szBuffer;
}

bool isAllDigit(string str)
{
	int i;
	for (i = 0; i != str.length(); i++)
	{
		if (!isdigit(str[i]))
		{
			return false;
		}
	}
	return true;
}

int test(void)
{
	int result;

	if (reader.parse(str, root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
	{
		std::string upload_id = root["uploadid"].asString();  // 访问节点，upload_id = "UP000000"  
		result = root["code"].asInt();    // 访问节点，code = 100
	}

	return result;
}

int test1(void)
{
	return 101;
}