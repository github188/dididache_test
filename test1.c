#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>

int main(int argc, char* argv[])
{
	CURL* curl = NULL;
	CURLcode res;

	// 1. 创建一个curl句柄
	curl = curl_easy_init();

	if(curl == NULL)
	{
		//perror("curl");
		exit(1);
	}
	// 2. 设置curl的模式和请求的url
	res = curl_easy_setopt(curl, CURLOPT_URL, "http://www.baidu.com");
	if(res != CURLE_OK)
	{
		perror("curl");
		exit(1);
	}

	// 3. 发送数据
	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		perror("curl");
		exit(1);
	}

	// 4. 接收数据

	// 5. 释放curl句柄
	curl_easy_cleanup(curl);

	return 0;
}
