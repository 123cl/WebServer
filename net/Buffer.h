#ifndef BUFFER_H
#define BUFFER_H

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string>
#include<string.h>
using namespace std;

class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize)
		 : buffer_(kCheapPrepend + initialSize),
		   readerIndex_(kCheapPrepend),writerIndex_(kCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == 0);
		assert(prependableBytes() == kCheapPrepend);
	} 

	//缓存区交换
	void swap(Buffer& rhs)
	{
		buffer_.swap(rhs.buffer_);
		std::swap(readerIndex_, rhs.readerIndex_);
		std::swap(writerIndex_, rhs.writerIndex_);
	}

	//返回读的指针
	const char* peek() const
	{
		return begin() + readerIndex_;
	}

	size_t readableBytes() const
	{
		return writerIndex_ - readerIndex_;
	}

	size_t writableBytes() const
	{
		return buffer_.size() - writerIndex_;
	}

	size_t prependableBytes() const
	{
		return readerIndex_;
	}

	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
		return crlf == beginWrite() ? NULL : crlf;
	}


	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if(len < readableBytes())
		{
			readerIndex_ += len;
		}
		else
		{
			retrieveAll();
		}
	}

	void retrieveAll()
	{
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
	}

	std::string retrieveAsString(size_t len)
	{
		assert(len <= readableBytes());
		string result(peek(), len);
		retrieve(len);
		return result;
	}

	string retrieveAllAsString()
  	{
    		return retrieveAsString(readableBytes());
  	}
	
	void append(const string& str)
	{
		append(str.data(), str.size());
	}
	void append(const char* data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWritten(len);
	}

	//确保缓存区可写空间足够，不足就扩充
	void ensureWritableBytes(size_t len)
	{
		if(writableBytes() < len) makeSpace(len);
		assert(writableBytes() >= len);
	}


	char* beginWrite()
  	{ return begin() + writerIndex_; }
		
	const char* beginWrite() const
  	{ return begin() + writerIndex_; }

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		writerIndex_ += len;
	}

	void unwrite(size_t len)
	{
		assert(len <= readableBytes());
		writerIndex_ -= len;
	}

	void prepend(const void* data, size_t len)
	{
		assert(len <= prependableBytes());
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readerIndex_);
	}


	//伸缩空间，保留reserve个字节
	void shrink(size_t reserve)
	{
		Buffer other;
		other.ensureWritableBytes(readableBytes()+reserve);
		other.append(peek(), readableBytes());
		swap(other);
	}

	ssize_t readFd(int fd, int* savedErrno);

private:
	char* begin()
	{
		return &*buffer_.begin();
	}

	const char* begin() const
  	{ return &*buffer_.begin(); }

	void makeSpace(size_t len)
	{
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
			buffer_.resize(writerIndex_+len);
		}
		else
		{
			assert(kCheapPrepend < readerIndex_);
			size_t readable = readableBytes();
			std::copy(begin()+readerIndex_, begin()+writerIndex_,begin()+kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}

	}


	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];
};



#endif
