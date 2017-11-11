#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <signal.h>
#include "cJSON.h"

#include <event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include <sys/queue.h>

#define MYHTTPD_SIGNATURE "MoCarHttpd v0.1"

// 当进程接收到SIGHUP/SIGTERM/SIGINT/SIGQUIT信号时, 终止程序的event循环监听事件
void httpd_handler(struct evhttp_request* req, void* arg)
{

}

void login_handler(struct evhttp_request* req, void* arg)
{
	// 1. 处理请求的数据 得到url和数据
	// 1.1 获取uri信息
	char* uri = (char*)evhttp_request_uri(req);
	printf("uri: %s\n", uri);

	// 1.2 获取数据信息
	size_t size = EVBUFFER_LENGTH(req->input_buffer);
	char* data = (char*)malloc(size);
	memcpy(data, (char*)EVBUFFER_DATA(req->input_buffer), size);
	printf("data: %s\n", data);
	free(data);

	// 2. 根据数据查询数据库中是否有该条记录

	// 3. 组织响应报文 发送给客户端
	// 3.1 组织头信息
	evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

	// 3.2 组织数据信息
	char* resData = "{ \"result\": \"ok\", \"recode\": 0 }";
	struct evbuffer* buf = evbuffer_new();
	evbuffer_add_printf(buf, "%s", resData);

	// 3.3 发送数据给客户端
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	printf("[response]: \n");
	printf("%s\n", resData);
}

void reg_handler(struct evhttp_request* req, void* arg)
{
	// 1. 处理请求的数据 得到url和数据
	// 1.1 获取uri信息
	char* uri = (char*)evhttp_request_uri(req);
	printf("uri: %s\n", uri);

	// 1.2 获取数据信息
	int size = EVBUFFER_LENGTH(req->input_buffer);
	char* data = (char*)malloc(size);
	memcpy(data, (char*)EVBUFFER_DATA(req->input_buffer), size);
	printf("data: %s\n", data);
	free(data);

	// 2. 根据数据查询数据库中是否有该条记录

	// 3. 组织响应报文 发送给客户端
	// 3.1 组织头信息
	evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

	// 3.2 组织数据信息
	char* resData = "{ \"result\": \"ok\", \"recode\": 0 }";
	struct evbuffer* buf = evbuffer_new();
	evbuffer_add_printf(buf, "%s", resData);

	// 3.3 发送数据给客户端
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	printf("[response]: \n");
	printf("%s\n", resData);

}

void signal_handler(int signum)
{
	switch(signum)
	{
	case SIGHUP:
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		event_loopbreak();
		break;
	}
}

// ./httpd_server -h 调出来帮助文档
void show_help()
{
    char *help = "http://localhost:8080\n"
        "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
        "-p <num>     port number to listen on, default is 1984\n"
        "-d           run as a deamon\n"
        "-t <second>  timeout for a http request, default is 120 seconds\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr,"%s",help);
}

void test1()
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
}

int main(int argc, char* argv[])
{
	// 信号退出并释放资源
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	// 默认参数
	int ret = 0;
	char* httpd_option_listen = "0.0.0.0";
	short httpd_option_port = 8090;
	int httpd_option_daemon = 0;
	int httpd_option_timeout = 120;

	// 根据输入的参数来设置监听ip port 是否为守护进程 等待时间
	while((ret = getopt(argc, argv, "l:p:dt:h")) != -1)
	{
		switch(ret)
		{
			case 'l':
				httpd_option_listen = optarg;
				break;
			case 'p':
				httpd_option_port = (int)optarg;
				break;
			case 'd':
				httpd_option_daemon = 1;
				break;
			case 't':
				httpd_option_timeout = (int)optarg;
			case 'h':
			default:
				show_help();
				exit(0);
		}
	}

	if(httpd_option_daemon)
	{
		pid_t pid = fork();
		if(pid < 0)
		{
			perror("fork");
			return pid;
		}
		else if(pid > 0)
		{
			// 父进程正常退出
			exit(0);
		}
	}
	
	// 初始化事件
	event_init();

	// 设置http的ip和port
	struct evhttp* httpd = (struct evhttp*)evhttp_start(httpd_option_listen, httpd_option_port);
	if(httpd == NULL)
	{
		perror("evhttp_start");
		return -1;
	}

	// 设置超时时间
	evhttp_set_timeout(httpd, httpd_option_timeout);

	// 将http事件上树
	evhttp_set_cb(httpd, "/", httpd_handler, NULL);
	evhttp_set_cb(httpd, "/login", login_handler, NULL);
	evhttp_set_cb(httpd, "/reg", reg_handler, NULL);

	// 循环处理监听事件
	event_dispatch();

	// 销毁http
	evhttp_free(httpd);

	return 0;
}
