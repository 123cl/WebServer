#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include "HttpServer.h"
#include "../base/Logging.h"
#include "EventLoop.h"
using namespace std;

int main()
{
	int IoThreads = 5, computeThreads = 10;

	EventLoop loop;
	HttpServer server(&loop, InetAddress(8000), "http_server", 8, IoThreads, computeThreads);

	server.start();
	loop.loop();

	return 0;	
}
