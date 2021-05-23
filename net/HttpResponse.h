#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include<map>
#include<string>
using std::string;

class Buffer;

//HTTP响应类的封装
class HttpResponse
{
public:
	enum HttpStatusCode
	{
		kUnknown,
		k200Ok = 200,
		k301MovedPermanently = 301,
		k400BadRequest = 400,
		k404NotFound = 404,
	};

	explicit HttpResponse(bool close) : statusCode_(kUnknown), closeConnection_(close)
	{

	}

	void setStatusCode(HttpStatusCode code)
	{
		statusCode_ = code;
	}

	void setStatusMessage(const string& message)
	{
		statusMessage_ = message;
	}

	void setCloseConnection(bool on)
	{
		closeConnection_ = on;
	}

	bool closeConnection() const
	{
		return closeConnection_;
	}

	//设置文档媒体类型
	void setContentType(const string& contentType)
	{
		addHeader("Content-Type", contentType);
	}

	void addHeader(const string& key, const string& value)
	{
		headers_[key] = value;
	}

	void setBody(const string& body)
	{
		body_ = body;
	}

	void appendToBuffer(Buffer* output) const;  //将HTTPResponse添加到Buffer

private:
	std::map<string, string> headers_;  //header列表
	HttpStatusCode statusCode_;       //状态响应码
	string statusMessage_;      //状态响应码对于的文本信息
	bool closeConnection_;      //是否关闭连接
	string body_;              //实体
};


#endif
