#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "../base/ThreadPool.h"
#include "TcpServer.h"
#include "Buffer.h"
#include "TimeWheel.h"
#include<unordered_set>
#include<boost/circular_buffer.hpp>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable
{
public:
	typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

	HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name,
		   int idleSeconds, int IoThreads, int computeThreads);

	EventLoop* getLoop() const
	{
		return server_.getLoop();
	}

	void start();

private:

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf);
	void onRequest(const TcpConnectionPtr&, const HttpRequest&);
 	void onTimer();

	int IoThreads_; //IO线程池
	int computeThreads_; //计算线程池
	ThreadPool threadPool_;
	TcpServer server_;
	HttpCallback httpCallback_;   //在处理HTTP请求（即调用onRequest）的过程中回调此函数，对请求进行具体的处理

	typedef std::unordered_set<EntryPtr> Bucket;  //环形缓冲区每个格子存放的是一个hash_set
	typedef boost::circular_buffer<Bucket> WeakConnectionList;  //环形缓冲区

	WeakConnectionList connectionBuckets_;  //连接队列环形缓冲区
};

#endif
