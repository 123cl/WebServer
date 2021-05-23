#ifndef CURREDNT_THREAD_H
#define CURREDNT_THREAD_H

namespace CurrentThread
{
	//__thread修饰的变量是线程局部存储的。
	extern __thread int t_cachedTid;   //线程真实pid的缓存
	extern __thread char t_tidString[32]; //tid的字符串表示形式
	extern __thread int t_tidStringLength;
	extern __thread const char* t_threadName;

	void cacheTid();
	inline int tid()
	{
		if (__builtin_expect(t_cachedTid == 0, 0))
		{
			cacheTid();
		}
		return t_cachedTid;
	}

	const char* tidString()
	{
		return t_tidString;
	}

	int tidStringLength() { return t_tidStringLength; }

	const char* name() { return t_threadName; }
}



#endif
