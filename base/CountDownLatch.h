#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H


#include "Condition.h"
#include "Mutex.h"

//确保Thread中传进去的func真的启动了以后，外层的start才返回

class CountDownLatch : noncopyable
{
public:
	explicit CountDownLatch(int count);

	void wait();  
	void countDown();  //计数器-1
	int getCount() const;

private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};





#endif

