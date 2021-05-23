#include "HttpResponse.h"
#include "Buffer.h"
#include <stdio.h>

void HttpResponse::appendToBuffer(Buffer* output) const
{
	char buf[32];

	//添加响应头
	snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
	output->append(buf);
	output->append(statusMessage_);
	output->append("\r\n");


	if (closeConnection_)
	{
		//如果是短链接，不需要告诉浏览器Content-length，浏览器也能正确处理
		output->append("Connection: close\r\n");
	}
	else
	{
		snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
		output->append(buf);
		output->append("Connection: Keep-Alive\r\n");
	}

	for (const auto& header : headers_)
	{
		output->append(header.first);
		output->append(": ");
		output->append(header.second);
		output->append("\r\n");
	}

	output->append("\r\n");  //header与实体之间的空行
	output->append(body_);
}
