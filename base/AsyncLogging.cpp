#include "AsyncLogging.h"
#include "LogFile.h"

AsyncLogging::AsyncLogging(const string& basename, int flushInterval)
	      : flushInterval_(flushInterval),
	        running_(false),
		basename_(basename),
		thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
		latch_(1),
		mutex_(),
		cond_(mutex_),
		currentBuffer_(new Buffer),
		nextBuffer_(new Buffer),
		buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	MutexLockGuard lock(mutex_);
	if(currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		//当前缓冲区已满，将当前缓冲区添加到待写入文件的已填满的缓冲区列表
		buffers_.push_back(std::move(currentBuffer_));

		if(nextBuffer_)
			currentBuffer_ = std::move(nextBuffer_);
		else
			currentBuffer_.reset(new Buffer); //前端写入速度太快，一下子把两个缓冲区写满了

		currentBuffer_->append(logline, len);
		cond_.notify(); //通知后端开始写日志
	}
}

void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_);

	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();

	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);

	while(running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{
			MutexLockGuard lock(mutex_);
			if(buffers_.empty())
			{
				cond_.waitForSeconds(flushInterval_);
			}

			buffers_.push_back(std::move(currentBuffer_));
			currentBuffer_ = std::move(newBuffer1); //将空闲的newbuffer置为当前缓冲区
			buffersToWrite.swap(buffers_);

			if(!nextBuffer_)
				nextBuffer_ = std::move(newBuffer2);
		}
		assert(!buffersToWrite.empty());

		//前端陷入死循环，拼命发送日志消息，超过后端的处理能力，会造成数据在内存中堆积
		if(buffersToWrite.size() > 25)
		{
			buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());  //丢掉多余日志，仅保留两块
		}

		for(auto& buffer : buffersToWrite)
			output.append(buffer->data(), buffer->length());

		if(buffersToWrite.size() > 2)
			buffersToWrite.resize(2);

		if(!newBuffer1)
		{
			assert(!buffersToWrite.empty());
			newBuffer1 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer1->reset();  //将缓冲区中的当前指针指向第一个地址
		}

                if(!newBuffer2)
                {
                        assert(!buffersToWrite.empty());
                        newBuffer2 = std::move(buffersToWrite.back());
                        buffersToWrite.pop_back();
                        newBuffer1->reset();  //将缓冲区中的当前指针指向第一个地址
                }

		buffersToWrite.clear();
		output.flush();
	}
	output.flush();
}







