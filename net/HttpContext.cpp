#include "Buffer.h"
#include "HttpContext.h"


bool HttpContext::processRequestLine(const char* begin, const char* end)
{
	bool succeed = false;
	const char* start = begin;
	const char* space = std::find(start, end, ' ');  //查找空格所在的位置

	if(space != end && request_.setMethod(start, space))
	{
		start = space + 1;
		space = std::find(start, end, ' ');
		if(space != end)
		{
			const char* question = std::find(start, space, '?');
			if(question != space)
			{
				request_.setPath(start, question);
				request_.setQuery(question, space);
			}
			else
			{
				request_.setPath(start, space);
			}

			start = space + 1;

			succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
			if(succeed)
			{
				if(*(end - 1) == '1')
				{
					request_.setVersion(HttpRequest::kHttp11);
				}
				else if(*(end - 1) == '0')
				{
					request_.setVersion(HttpRequest::kHttp10);
				}
				else 
				{
					succeed = false;
				}
			}
		}
	}
	return succeed;
}


bool HttpContext::parseRequest(Buffer* buf)
{
	bool ok = true;
	bool hasMore = true;

	while(hasMore)
	{
		if (state_ == kExpectRequestLine)    //处于解析请求行的状态
		{
			const char* crlf = buf->findCRLF(buf->peek());

			if(crlf)
			{
				ok = processRequestLine(buf->peek(), crlf);  //解析请求行
				if(ok)
				{
					buf->retrieve(crlf + 2 - (buf->peek()));
					state_ = kExpectHeaders;
				}
				else
				{
					hasMore = false;
				}
			}
			else
			{
				hasMore = false;
			}
		}
		else if (state_ == kExpectHeaders)  //解析header
		{
			const char* crlf = buf->findCRLF(buf->peek());
			if(crlf)
			{
				const char* colon = std::find(buf->peek(), crlf, ':');  //:所在的位置
				if(colon != crlf)
				{
					request_.addHeader(buf->peek(), colon, crlf);
				}
				else
				{
					state_ = kGotAll;
					hasMore = false;
				}
				buf->retrieve(crlf + 2 - (buf->peek()));
			}
			else
			{
				hasMore = false;
			}
		}
		else if(state_ == kExpectBody)
		{

		}
	}

	return ok;

}
