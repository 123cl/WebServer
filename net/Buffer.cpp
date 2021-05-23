#include "Buffer.h"
#include "SocketsOps.h"

#include <errno.h>
#include <sys/uio.h>

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;


//从套接字读取数据，添加到当前缓冲区中
//结合栈上的空间，避免内存使用过大，提高内存使用率
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();

	vec[0].iov_base = begin()+writerIndex_;
	vec[0].iov_len = writable;

	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;

	const ssize_t n = sockets::readv(fd, vec, 2);
	if(n < 0) *savedErrno = errno;
	else if(n <= writable)
	{
		writerIndex_ += n;
	}
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return n;
}






































