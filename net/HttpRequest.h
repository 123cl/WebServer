#ifndef HTTP_HTTPREQUEST_H
#define HTTP_HTTPREQUEST_H

#include <map>
#include <assert.h>
#include <stdio.h>
#include<string>
using std::string;

//HTTP请求的封装
class HttpRequest 
{
public:
	enum Method
	{
		kInvalid, kGet, kPost, kHead, kPut, kDelete
	};

	enum Version
	{
		kUnknown, kHttp10, kHttp11
	};

	HttpRequest() : method_(kInvalid), version_(kUnknown)
	{

	}

	void setVersion(Version v)
	{
		version_ = v;
	}

	Version getVersion() const
	{
		return version_;
	}

	bool setMethod(const char* start, const char* end)
	{
		assert(method_ == kInvalid);
		string m(start, end);

		if(m == "GET") method_ = kGet;
		else if(m == "POST") method_ = kPost;
		else if(m == "HEAD") method_ = kHead;
		else if(m == "PUT") method_ = kPut;
		else if(m == "DELETE") method_ = kDelete;
		else method_ = kInvalid;

		return method_ != kInvalid;
	}

	Method method() const
	{
		return method_;
	}

	//请求方法转换为字符串
	const char* methodString() const
	{
		const char* result = "UNKNOWN";
		switch(method_)
		{
			case kGet:
				result = "GET";
				break;
			case kPost:
				result = "POST";
				break;
			case kHead:
				result = "HEAD";
				break;
			case kPut:
				result = "PUT";
				break;
			case kDelete:
				result = "DELETE";
				break;
			default:
				break;
		}
		return result;
	}

	void setPath(const char* start, const char* end)
	{
		path_.assign(start, end);
	}

	const string& path() const
	{
		return path_;
	}

	void setQuery(const char* start, const char* end)
	{
		query_.assign(start, end);
	}

	const string& query() const
  	{ return query_; }
	
	//添加头部信息
	void addHeader(const char* start, const char* colon, const char* end)
	{	
    		string field(start, colon);  //header域
    		++colon;
    		//去除左空格
    		while (colon < end && isspace(*colon))
    		{
      			++colon;
    		}
    		string value(colon, end);  //header值
    		//去除右空格
    		while (!value.empty() && isspace(value[value.size()-1]))
    		{
      			value.resize(value.size()-1);
    		}
    		headers_[field] = value;
  	}

	string getHeader(const string& field) const
	{
		string result;
		std::map<string, string>::const_iterator it = headers_.find(field);

		if(it != headers_.end())
		{
			result = it->second;
		}
		return result;
	}

	const std::map<string, string>& headers() const
	{
		return headers_;
	}

	void swap(HttpRequest& that)
  	{
    		std::swap(method_, that.method_);
    		std::swap(version_, that.version_);
    		path_.swap(that.path_);
   	 	query_.swap(that.query_);
    		headers_.swap(that.headers_);
  	}

private:
	Method method_;
	Version version_;
	string path_;
	string query_;
	std::map<string, string> headers_;
};

#endif
