#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include "HttpRequest.h"
#include<string>
using namespace::std;

class Buffer;

//协议解析类
class HttpContext
{
public:
	enum HttpRequestParseState
	{
		kExpectRequestLine,  //正处于解析请求行
		kExpectHeaders,     //解析头部信息
		kExpectBody,   //解析实体
		kGotAll,   //解析完毕
	};

	HttpContext() : state_(kExpectRequestLine)
	{

	}

	bool parseRequest(Buffer* buf);

	void reset()
	{
		state_ = kExpectRequestLine;
		HttpRequest dummy;
		request_.swap(dummy);
	}

	const HttpRequest& request() const
	{
		return request_;
	}
	bool gotAll() const
  	{ return state_ == kGotAll; }
private:
	bool processRequestLine(const char* begin, const char* end);

	HttpRequestParseState state_;  //请求解析状态
	HttpRequest request_;
};


#endif
