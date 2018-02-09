#include "stdafx.h"
#include "service_sdk.h"
#include "json/json.h"

const char* str = "{\"uploadid\": \"UP000000\",\"code\": 100,\"msg\": \"\",\"files\": \"\"}";

Json::Reader reader;
Json::Value root;

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