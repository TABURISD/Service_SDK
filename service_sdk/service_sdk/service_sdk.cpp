#include "stdafx.h"
#include "service_sdk.h"
#include "json/json.h"

const char* str = "{\"uploadid\": \"UP000000\",\"code\": 100,\"msg\": \"\",\"files\": \"\"}";

Json::Reader reader;
Json::Value root;

int test(void)
{
	int result;

	if (reader.parse(str, root))  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{
		std::string upload_id = root["uploadid"].asString();  // ���ʽڵ㣬upload_id = "UP000000"  
		result = root["code"].asInt();    // ���ʽڵ㣬code = 100
	}

	return result;
}

int test1(void)
{
	return 101;
}