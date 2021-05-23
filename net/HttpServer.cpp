#include<cstring>
#include<algorithm>
#include "HttpServer.h"
#include "../base/Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TimeWheel.h"
#include<string>
#include<iostream>
#include<functional>
#include "EventLoop.h"
using namespace std;

//网站的根目录
const char* doc_root = "/home/chen/WebServer/root";

bool benchmark = false;

void HandleRequest(const HttpRequest& req, HttpResponse* resp)
{
	std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
	char* my_real_file;
	strcpy(my_real_file, doc_root);

	if(req.path() == "/")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");

	}


}


HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, int idleSeconds, int IoThreads, int computeThreads)
	  : server_(loop, listenAddr, name),
	    IoThreads_(IoThreads), computeThreads_(computeThreads),
	    httpCallback_(HandleRequest),
	    connectionBuckets_(idleSeconds)
{
	server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this,std::placeholders::_1));
	server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
	server_.setThreadNum(IoThreads_);

	loop->startTimer(1.0, std::bind(&HttpServer::onTimer, this));
}

void HttpServer::start()
{
	threadPool_.start(computeThreads_);
	server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
	if(conn->connected())
	{
		EntryPtr entry(new Entry(conn));
		connectionBuckets_.back().insert(entry);
		WeakEntryPtr weakEntry(entry);
		conn->settimerContext(weakEntry);
		
		conn->sethttpContext(HttpContext());
	}
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
	HttpContext* context = boost::any_cast<HttpContext>(conn->getMutablehttpContext());

	if (!context->parseRequest(buf))
	{
		string cur = "HTTP/1.1 400 Bad Request\r\n\r\n";
		conn->send(cur.data(), cur.size());
		conn->shutdown();
	}

	//请求消息解析完毕
	if(context->gotAll())
	{
		onRequest(conn, context->request());
		context->reset();  //本次请求处理完毕，重置HTTPCONTEXT，适用于长连接
	}

	WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->gettimerContext()));//全都加到其中，不影响短连接
	EntryPtr entry(weakEntry.lock());
	if (entry)
	{
		connectionBuckets_.back().insert(entry);
	}
}

void HttpServer::onTimer()
{
	connectionBuckets_.push_back(Bucket()); //相当于把tail位置原来的桶清空了 ，然后增加了一个空的桶
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
	const string& connection = req.getHeader("Connection");
	bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
	
	HttpResponse response(close);
	httpCallback_(req, &response);

	Buffer buf;
	response.appendToBuffer(&buf);
	conn->send(&buf);

	if (response.closeConnection())
	{
		conn->shutdown();
	}
}








