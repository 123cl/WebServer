#ifndef THREAD_H
#define THREAD_H


#include<functional>
#include<memory>
#include<string>
#include<pthread.h>
#include "CountDownLatch.h"
using namespace std;

class Thread : noncopyable
{
public:
	typedef std::function<void()> ThreadFunc;

	explicit Thread(ThreadFunc, const string &name = string());
	~Thread();

	void start();
	int join();

	bool started() const {return started_;}
	pid_t tid() const { return tid_; }
	const string& name() const { return name_; }

private:
	void setDefaultName();

	bool started_;
	bool joined_;
	pthread_t pthreadId_;
	pid_t tid_;
	ThreadFunc func_;
	string name_;
	CountDownLatch latch_;
};

#endif
