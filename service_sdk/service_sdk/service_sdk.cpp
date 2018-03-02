#include "stdafx.h"
#include "service_sdk.h"

static void UTF8toANSI(string &strUTF8);
static void ANSItoUTF8(string &strAnsi);
static bool isAllDigit(string str);
static int str_to_hex(char *string, wchar_t *cbuf, int len);
static string string_to_hex(const string& str);

static string username_now, password_now;
static bool islogin = true;
static int user_group;//用户权限组
static int user_role;
static const std::string SERVER_ADDRESS("tcp://139.196.136.90:1883");
static const auto TIMEOUT = std::chrono::seconds(10);
static const int N_RETRY_ATTEMPTS = 5;
static bool callback_flag = false;
static class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
	}

	void on_success(const mqtt::token& tok) override {
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};
static class callback : public virtual mqtt::callback,
	public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// Re-connection success
	void on_success(const mqtt::token& tok) override {
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		msg_ret = msg;
		callback_flag = true;
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	mqtt::const_message_ptr msg_ret;
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
		: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

///////////////////////////////////////

string login(string username,string password,string myControllerId)//用户登录
{
	if ((username.length() != 6) || (password.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	
	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();

	string temp = username;
	string strFormData = "Username=" + temp + "&Password=" + passwordval;

	bool httpresult;

	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/wapi/auth/session"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult=pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if (root["ErrCode"].asInt() == 0)
	{
		int i = 0;
		user_group = root["Result"]["Groups"][i]["Group"].asInt();
		user_role = root["Result"]["Groups"][i]["UserRole"].asInt();

		username_now = username;
		password_now = password;
		islogin = true;

		return "{ \"ErrCode\":0,\"ErrMsg\":\"登陆成功\"}";
	} else {
		return result;
	}
}

string logout(void)
{
	bool httpresult;

	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_DELETE, _T("/wapi/auth/session"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, "", 0);

	if (!httpresult)
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if (root["ErrCode"].asInt() == 0)
	{
		username_now = "";
		password_now = "";
		islogin = false;

		return "{ \"ErrCode\":0,\"ErrMsg\":\"登出成功\"}";
	}
	else {
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
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if ((result.find("20000001") != string::npos) && (islogin))
	{
		login(username_now, password_now, password_now);

		bool httpresult;

		string strFormData = "";
		CInternetSession session(_T("session"));
		INTERNET_PORT nPort = 10001;
		CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
		CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, _T("/wapi/gps/roads"));
		pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
		httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

		if (!httpresult)
			return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		char szBuff[2048];
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
			return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
		}
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
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if ((result.find("20000001") != string::npos) && (islogin))
	{
		bool httpresult;

		string temp, strdata;
		strdata = "/wapi/bollard/bollards?Group=" + to_string(user_group);
		string strFormData = "";
		CInternetSession session(_T("session"));
		INTERNET_PORT nPort = 10001;
		CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
		CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, (CString)strdata.c_str());
		pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
		httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

		if (!httpresult)
			return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		char szBuff[4096];
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
			return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
		}
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
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if ((result.find("20000001") != string::npos) && (islogin))
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
			return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		char szBuff[4096];
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
			return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
		}
	}

	return result;
}

string addUser(string username, string password, string usernick, string role)
{
	if ((username.length() != 6) || (password.length() != 6) || (role.length() != 3))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字、权限应为3位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)) || (!isAllDigit(role)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号、密码、权限应为数字\"}";

	bool httpresult;

	string strFormData = "Username=" + username + "&Password=" + password + "&Nick=" + usernick + "&Group=" + to_string(user_group) + "&Role=" + role;
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/wapi/group/users"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if ((result.find("20000001") != string::npos) && (islogin))
	{
		bool httpresult;

		string strFormData = "Username=" + username + "&Password=" + password + "&Nick=" + usernick + "&Group=" + to_string(user_group) + "&Role=" + role;
		CInternetSession session(_T("session"));
		INTERNET_PORT nPort = 10001;
		CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
		CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, _T("/wapi/group/users"));
		pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
		httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

		if (!httpresult)
			return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		char szBuff[2048];
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
			return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
		}
	}


	return result;
}

string deleteUser(string username)
{
	if ((username.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号应为6位数字\"}";
	if ((!isAllDigit(username)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号应为6位数字\"}";

	bool httpresult;

	string strFormData = "Group=" + to_string(user_group) + "&Username=" + username;
	CInternetSession session(_T("session"));
	INTERNET_PORT nPort = 10001;
	CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
	CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_DELETE, _T("/wapi/group/users"));
	pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
	httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

	if (!httpresult)
		return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
		return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
	}

	if ((result.find("20000001") != string::npos) && (islogin))
	{
		bool httpresult;

		string strFormData = "Group=" + to_string(user_group) + "&Username=" + username;
		CInternetSession session(_T("session"));
		INTERNET_PORT nPort = 10001;
		CHttpConnection* pHttpConnect = session.GetHttpConnection(_T("zjt.iotcloudsoft.com"), nPort);
		CHttpFile* pFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_DELETE, _T("/wapi/group/users"));
		pFile->AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded"));
		httpresult = pFile->SendRequest(NULL, 0, (LPVOID)(LPCTSTR)strFormData.c_str(), strFormData.size());

		if (!httpresult)
			return "{ \"ErrCode\":20000017,\"ErrMsg\":\"服务器连接失败\"}";

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
			return "{ \"ErrCode\":20000016,\"ErrMsg\":\"服务器信息无法解析\"}";
		}
	}

	return result;
}

string mqttAskSystem(string username, string password, string deviceid, SystemState* systemstate)
{
	string temp;
	time_t start, stop;
	const char * payload = { "{\"Type\":\" ASKSYS\"}" };

	if ((username.length() != 6) || (password.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	Json::Reader reader;//解析道路信息
	Json::Value root;

	string json = cb.msg_ret->to_string();
	reader.parse(json, root, false);


	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	string sysstate = root["Str"].asString();

	systemstate->timing[0] = (sysstate[3] - 0x30) * 10 + (sysstate[4] - 0x30);
	systemstate->timing[1] = (sysstate[6] - 0x30) * 10 + (sysstate[7] - 0x30);
	systemstate->timing[2] = (sysstate[11] - 0x30) * 10 + (sysstate[12] - 0x30);
	systemstate->timing[3] = (sysstate[14] - 0x30) * 10 + (sysstate[15] - 0x30);
	systemstate->timing[4] = (sysstate[18] - 0x30) * 10 + (sysstate[19] - 0x30);
	systemstate->timing[5] = (sysstate[21] - 0x30) * 10 + (sysstate[22] - 0x30);
	systemstate->timing[6] = (sysstate[26] - 0x30) * 10 + (sysstate[27] - 0x30);
	systemstate->timing[7] = (sysstate[29] - 0x30) * 10 + (sysstate[30] - 0x30);
	systemstate->timing[8] = (sysstate[33] - 0x30) * 10 + (sysstate[34] - 0x30);
	systemstate->timing[9] = (sysstate[36] - 0x30) * 10 + (sysstate[37] - 0x30);
	systemstate->timing[10] = (sysstate[41] - 0x30) * 10 + (sysstate[42] - 0x30);
	systemstate->timing[11] = (sysstate[44] - 0x30) * 10 + (sysstate[45] - 0x30);

	systemstate->immediately[0] = (sysstate[51] - 0x30) * 10 + (sysstate[52] - 0x30);
	systemstate->immediately[1] = (sysstate[54] - 0x30) * 10 + (sysstate[55] - 0x30);

	systemstate->lighttype = LightType(sysstate[61] - 0x30);
	systemstate->modetype = ModeType(sysstate[62] - 0x30);

	systemstate->music = (sysstate[63] - 0x30) * 10 + (sysstate[64] - 0x30);

	systemstate->time[0] = (sysstate[68] - 0x30) * 10 + (sysstate[69] - 0x30);
	systemstate->time[1] = (sysstate[71] - 0x30) * 10 + (root["Str"].asString()[72] - 0x30);

	return "{ \"ErrCode\":0,\"Result\":\"获取成功\"}";
}

string mqttAskList(string username, string password, string deviceid)
{
	time_t start, stop;
	const char * payload = { "{\"Type\":\" ASKLIST\"}" };

	if ((username.length() != 6) || (password.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	Json::Reader reader;//解析道路信息
	Json::Value root;

	string json = cb.msg_ret->to_string();
	reader.parse(json, root, false);


	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	unsigned int listcount;//取列表大小
	listcount = atoi(root["Num"].asString().c_str());///////////////////////////////////////////////////////////////////////////

	char *templist[48];
	string tempstr = root["Str"].asString();
	for (unsigned int i = 0; i < tempstr.length(); i++)
	{
		tempstr[i] = tempstr[i + 1];
	}
	char* tempptr = const_cast<char*>(tempstr.c_str());
	//tempptr = tempptr + sizeof(char);
	templist[0] = strtok(tempptr, "#");//取列表内容

	for (unsigned int i = 0; i < listcount; i++)
	{
		templist[i + 1] = strtok(NULL, "#");
	}

	for (unsigned int i = 0; i < listcount; i++)
	{
		char swap;
		for (unsigned int j = 0; j < 16; j = j + 4)
		{
			swap = templist[i][j];
			templist[i][j] = templist[i][j + 2];
			templist[i][j + 2] = swap;

			swap = templist[i][j + 1];
			templist[i][j + 1] = templist[i][j + 3];
			templist[i][j + 3] = swap;
		}
	}
	string strret = "{\"ErrCode\":0,\"Result\":[";
	for (unsigned int i = 0; i < listcount; i++)
	{
		strret += "{\"Name\":\"";
		strret += templist[i];
		if (i < listcount - 1)
			strret += "\"},";
		else
			strret += "\"}";
	}
	strret += "]}";

	return strret;
}

string mqttSetTTS(string username, string password, string deviceid, string gbk)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	if ((username.length() != 6) || (password.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if (gbk.length()>76)
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"输入内容过长\"}";

	temppayload = { "{\"Type\":\"SETTTS\",\"Str\":\"" + string_to_hex(gbk) + "\"}" };
	payload = temppayload.c_str();

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (temppayload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

string mqttSetPTT(string username, string password, string deviceid)
{
	time_t start, stop;
	const char * payload = { "{\"Type\":\"SETPTT\"}" };

	if ((username.length() != 6) || (password.length() != 6))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";
	if ((!isAllDigit(username)) || (!isAllDigit(password)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"账号与密码应为6位数字\"}";

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

/*string mqttSetSYS(string username, string password, string deviceid, int8_t *timing, int8_t *immediately, LightType lighttype, ModeType modetype, MusicList music)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0000,T:00h00m\"}" };
	payload = temppayload.c_str();

	for (unsigned char i = 0; i < 6; i++)
	{
		if (!((timing[2 * i] >= 0) && (timing[2 * i] <= 23)))
			return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
		if (!((timing[2 * i + 1] >= 0) && (timing[2 * i + 1] <= 59)))
			return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	}

	int16_t timing1, timing2, timing3, timing4, timing5, timing6;

	timing1 = timing[0] * 60 + timing[1];
	timing2 = timing[2] * 60 + timing[3];
	timing3 = timing[4] * 60 + timing[5];
	timing4 = timing[6] * 60 + timing[7];
	timing5 = timing[8] * 60 + timing[9];
	timing6 = timing[10] * 60 + timing[11];

	if ((!((timing1 == 0) && (timing2 == 0))) && (timing2 < timing1))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((!((timing3 == 0) && (timing4 == 0))) && (timing4 < timing3))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((!((timing5 == 0) && (timing6 == 0))) && (timing6 < timing5))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((modetype == Timing) && (timing1 == 0) && (timing2 == 0) && (timing3 == 0) && (timing4 == 0) && (timing5 == 0) && (timing6 == 0))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"Timing模式下timing参数不能全部为0\"}";

	if (!((immediately[0] >= 0) && (immediately[0] <= 23)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"immediately参数格式错误\"}";
	if (!((immediately[1] >= 0) && (immediately[1] <= 59)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"immediately参数格式错误\"}";
	if ((modetype == Immediately) && (immediately[0] == 0) && (immediately[1] == 0))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"Immediately模式下immediately参数不能为0\"}";

	{
		temppayload[27] = timing[0] / 10 + 0x30;
		temppayload[28] = timing[0] % 10 + 0x30;
		temppayload[30] = timing[1] / 10 + 0x30;
		temppayload[31] = timing[1] % 10 + 0x30;

		temppayload[35] = timing[2] / 10 + 0x30;
		temppayload[36] = timing[2] % 10 + 0x30;
		temppayload[38] = timing[3] / 10 + 0x30;
		temppayload[39] = timing[3] % 10 + 0x30;

		temppayload[42] = timing[4] / 10 + 0x30;
		temppayload[43] = timing[4] % 10 + 0x30;
		temppayload[45] = timing[5] / 10 + 0x30;
		temppayload[46] = timing[5] % 10 + 0x30;

		temppayload[50] = timing[6] / 10 + 0x30;
		temppayload[51] = timing[6] % 10 + 0x30;
		temppayload[53] = timing[7] / 10 + 0x30;
		temppayload[54] = timing[7] % 10 + 0x30;

		temppayload[57] = timing[8] / 10 + 0x30;
		temppayload[58] = timing[8] % 10 + 0x30;
		temppayload[60] = timing[9] / 10 + 0x30;
		temppayload[61] = timing[9] % 10 + 0x30;

		temppayload[65] = timing[10] / 10 + 0x30;
		temppayload[66] = timing[10] % 10 + 0x30;
		temppayload[68] = timing[11] / 10 + 0x30;
		temppayload[69] = timing[11] % 10 + 0x30;

		temppayload[75] = immediately[0] / 10 + 0x30;
		temppayload[76] = immediately[0] % 10 + 0x30;
		temppayload[78] = immediately[1] / 10 + 0x30;
		temppayload[79] = immediately[1] % 10 + 0x30;

		temppayload[85] = lighttype + 0x30;
		temppayload[86] = modetype + 0x30;
		temppayload[87] = music.id / 10 + 0x30;
		temppayload[88] = music.id % 10 + 0x30;


		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}*/

string mqttSetDeviceTiming(string username, string password, string deviceid, int8_t *timing, LightType lighttype, MusicList music)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0200,T:00h00m\"}" };
	payload = temppayload.c_str();

	for (unsigned char i = 0; i < 6; i++)
	{
		if (!((timing[2 * i] >= 0) && (timing[2 * i] <= 23)))
			return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
		if (!((timing[2 * i + 1] >= 0) && (timing[2 * i + 1] <= 59)))
			return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	}

	int16_t timing1, timing2, timing3, timing4, timing5, timing6;

	timing1 = timing[0] * 60 + timing[1];
	timing2 = timing[2] * 60 + timing[3];
	timing3 = timing[4] * 60 + timing[5];
	timing4 = timing[6] * 60 + timing[7];
	timing5 = timing[8] * 60 + timing[9];
	timing6 = timing[10] * 60 + timing[11];

	if ((!((timing1 == 0) && (timing2 == 0))) && (timing2 < timing1))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((!((timing3 == 0) && (timing4 == 0))) && (timing4 < timing3))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((!((timing5 == 0) && (timing6 == 0))) && (timing6 < timing5))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"timing参数格式错误\"}";
	if ((timing1 == 0) && (timing2 == 0) && (timing3 == 0) && (timing4 == 0) && (timing5 == 0) && (timing6 == 0))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"Timing模式下timing参数不能全部为0\"}";

	{
		temppayload[27] = timing[0] / 10 + 0x30;
		temppayload[28] = timing[0] % 10 + 0x30;
		temppayload[30] = timing[1] / 10 + 0x30;
		temppayload[31] = timing[1] % 10 + 0x30;

		temppayload[35] = timing[2] / 10 + 0x30;
		temppayload[36] = timing[2] % 10 + 0x30;
		temppayload[38] = timing[3] / 10 + 0x30;
		temppayload[39] = timing[3] % 10 + 0x30;

		temppayload[42] = timing[4] / 10 + 0x30;
		temppayload[43] = timing[4] % 10 + 0x30;
		temppayload[45] = timing[5] / 10 + 0x30;
		temppayload[46] = timing[5] % 10 + 0x30;

		temppayload[50] = timing[6] / 10 + 0x30;
		temppayload[51] = timing[6] % 10 + 0x30;
		temppayload[53] = timing[7] / 10 + 0x30;
		temppayload[54] = timing[7] % 10 + 0x30;

		temppayload[57] = timing[8] / 10 + 0x30;
		temppayload[58] = timing[8] % 10 + 0x30;
		temppayload[60] = timing[9] / 10 + 0x30;
		temppayload[61] = timing[9] % 10 + 0x30;

		temppayload[65] = timing[10] / 10 + 0x30;
		temppayload[66] = timing[10] % 10 + 0x30;
		temppayload[68] = timing[11] / 10 + 0x30;
		temppayload[69] = timing[11] % 10 + 0x30;

		temppayload[85] = lighttype + 0x30;
		temppayload[87] = music.id / 10 + 0x30;
		temppayload[88] = music.id % 10 + 0x30;


		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

string mqttSetDeviceStop(string username, string password, string deviceid)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0300,T:00h00m\"}" };
	payload = temppayload.c_str();

	{
		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

string mqttSetDeviceTrigger(string username, string password, string deviceid, LightType lighttype, MusicList music)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0400,T:00h00m\"}" };
	payload = temppayload.c_str();

	{
		temppayload[85] = lighttype + 0x30;
		temppayload[87] = music.id / 10 + 0x30;
		temppayload[88] = music.id % 10 + 0x30;


		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

string mqttSetDeviceContinuity(string username, string password, string deviceid, LightType lighttype, MusicList music)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0100,T:00h00m\"}" };
	payload = temppayload.c_str();


	{
		temppayload[85] = lighttype + 0x30;
		temppayload[87] = music.id / 10 + 0x30;
		temppayload[88] = music.id % 10 + 0x30;


		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

string mqttSetDeviceImmediately(string username, string password, string deviceid, int8_t *immediately, LightType lighttype, MusicList music)
{
	time_t start, stop;
	const char * payload;
	string temppayload;

	temppayload = { "{\"Type\":\"SETSYS\",\"Str\":\"FP:00h00mto00h00m,00h00mto00h00m,00h00mto00h00m,IP:00h00m,AL:0000,T:00h00m\"}" };
	payload = temppayload.c_str();


	if (!((immediately[0] >= 0) && (immediately[0] <= 23)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"immediately参数格式错误\"}";
	if (!((immediately[1] >= 0) && (immediately[1] <= 59)))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"immediately参数格式错误\"}";
	if ((immediately[0] == 0) && (immediately[1] == 0))
		return "{ \"ErrCode\":20000015,\"ErrMsg\":\"Immediately模式下immediately参数不能全为0\"}";

	{
		temppayload[75] = immediately[0] / 10 + 0x30;
		temppayload[76] = immediately[0] % 10 + 0x30;
		temppayload[78] = immediately[1] / 10 + 0x30;
		temppayload[79] = immediately[1] % 10 + 0x30;

		temppayload[85] = lighttype + 0x30;
		temppayload[87] = music.id / 10 + 0x30;
		temppayload[88] = music.id % 10 + 0x30;


		time_t temptime = time(NULL);
		struct tm *now_time = localtime(&temptime);
		temppayload[92] = now_time->tm_hour / 10 + 0x30;
		temppayload[93] = now_time->tm_hour % 10 + 0x30;
		temppayload[95] = now_time->tm_min / 10 + 0x30;
		temppayload[96] = now_time->tm_min % 10 + 0x30;
	}

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(10);
	connOpts.set_clean_session(true);
	connOpts.set_user_name("1#" + username);

	MD5 md5;//加密用户密码
	md5.update(password);
	string passwordval = md5.toString();
	md5.reset();
	connOpts.set_password(passwordval);

	mqtt::async_client client(SERVER_ADDRESS, "1#" + username);
	action_listener subListener_("Subscription");
	callback cb(client, connOpts);
	client.set_callback(cb);

	try {
		client.connect(connOpts)->wait_for(TIMEOUT);
		client.subscribe("device/info/" + deviceid, 1)->wait_for(TIMEOUT);
		client.publish("command/control/" + deviceid, payload, strlen(payload), 1, false)->wait_for(TIMEOUT);
	}
	catch (const mqtt::exception&) {
		return "{ \"ErrCode\":20000018,\"ErrMsg\":\"MQTT服务器连接失败\"}";
	}

	start = time(NULL);
	while (true)
	{
		if (callback_flag == true)
			break;
		stop = time(NULL);
		if ((stop - start) >= 10)
			return "{ \"ErrCode\":20000019,\"ErrMsg\":\"MQTT连接超时\"}";
	}
	callback_flag = false;

	client.unsubscribe("device/info/" + deviceid)->wait_for(TIMEOUT);
	client.disconnect()->wait_for(TIMEOUT);

	if (payload == cb.msg_ret->to_string())
		return "{ \"ErrCode\":0,\"ErrMsg\":\"设置成功\"}";
	else
		return "{ \"ErrCode\":20000020 ,\"ErrMsg\":\"MQTT设置失败\"}";
}

vector <MusicList> handleMusicList(string MusicListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	MusicList temp;
	wchar_t tempwchar[5];

	reader.parse(MusicListInfo, root, false);

	vector <MusicList> templist(0);
	int size = root["Result"].size();

	for (int i = 0; i<size; i++)
	{
		temp.id = i + 1;
		str_to_hex(const_cast<char*>(root["Result"][i]["Name"].asString().c_str()), tempwchar, 16);
		temp.name = tempwchar;

		templist.push_back(temp);
	}

	temp.id = 29;
	temp.name = L"警音快";
	templist.push_back(temp);
	temp.id = 30;
	temp.name = L"警音慢";
	templist.push_back(temp);
	temp.id = 31;
	temp.name = L"录音播放";
	templist.push_back(temp);
	temp.id = 32;
	temp.name = L"文字播放";
	templist.push_back(temp);
	temp.id = 0;
	temp.name = L"关闭警音";
	templist.push_back(temp);

	return templist;
}

vector <Roadlist> handleRoadList(string RoadListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	Roadlist temp;

	reader.parse(RoadListInfo, root, false);

	vector <Roadlist> templist(0);
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

vector <DeviceList> handleDeviceList(string DeviceListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	DeviceList temp;

	reader.parse(DeviceListInfo, root, false);

	vector <DeviceList> templist(0);
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

vector <UserList> handleUserList(string UserListInfo)
{
	Json::Reader reader;//解析道路信息
	Json::Value root;
	UserList temp;

	reader.parse(UserListInfo, root, false);

	vector <UserList> templist(0);
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


/////////////////static function//////////////

static void UTF8toANSI(string &strUTF8)
{
	//获取转换为多字节后需要的缓冲区大小,创建多字节缓冲区
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

static void ANSItoUTF8(string &strAnsi)
{
	//获取转换为宽字节后需要的缓冲区大小,创建宽字节缓冲区,936为简体中文GB2312代码页  
	UINT nLen = MultiByteToWideChar(936, NULL, strAnsi.c_str(), -1, NULL, NULL);
	WCHAR *wszBuffer = new WCHAR[nLen + 1];
	nLen = MultiByteToWideChar(936, NULL, strAnsi.c_str(), -1, wszBuffer, nLen);
	wszBuffer[nLen] = 0;
	//获取转为UTF8多字节后需要的缓冲区大小,创建多字节缓冲区  
	nLen = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, NULL, NULL, NULL, NULL);
	CHAR *szBuffer = new CHAR[nLen + 1];
	nLen = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, szBuffer, nLen, NULL, NULL);
	szBuffer[nLen] = 0;

	strAnsi = szBuffer;
	//内存清理  
	delete[]wszBuffer;
	delete[]szBuffer;
}

static bool isAllDigit(string str)
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

static int str_to_hex(char *string, wchar_t *cbuf, int len)
{
	unsigned char first, second, third, forth;
	int idx, ii = 0;

	for (idx = 0; idx<len; idx += 4)
	{
		first = string[idx];
		second = string[idx + 1];
		third = string[idx + 2];
		forth = string[idx + 3];

		if (first >= '0' && first <= '9')
			first = first - '0';
		else if (first >= 'A' && first <= 'F')
			first = first - 'A' + 10;
		else if (first >= 'a' && first <= 'f')
			first = first - 'a' + 10;
		else
			return -1;

		if (second >= '0' && second <= '9')
			second = second - '0';
		else if (second >= 'A' && second <= 'F')
			second = second - 'A' + 10;
		else if (second >= 'a' && second <= 'f')
			second = second - 'a' + 10;
		else
			return -1;

		if (third >= '0' && third <= '9')
			third = third - '0';
		else if (third >= 'A' && third <= 'F')
			third = third - 'A' + 10;
		else if (third >= 'a' && third <= 'f')
			third = third - 'a' + 10;
		else
			return -1;

		if (forth >= '0' && forth <= '9')
			forth = forth - '0';
		else if (forth >= 'A' && forth <= 'F')
			forth = forth - 'A' + 10;
		else if (forth >= 'a' && forth <= 'f')
			forth = forth - 'a' + 10;
		else
			return -1;

		cbuf[ii++] = first << 12 | second << 8 | third << 4 | forth;
	}

	cbuf[ii++] = 0;

	return 0;
}

static string string_to_hex(const string& str) //transfer string to hex-string  
{
	string result = "";
	string tmp;
	stringstream ss;

	for (int i = 0; i<str.size(); i++)
	{
		ss << hex << int(str[i]) << endl;
		ss >> tmp;
		result += tmp.substr(tmp.length() - 2, 2);
	}

	return result;
}