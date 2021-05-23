#ifndef TIME_WHEEL_H
#define TIME_WHEEL_H

#include<memory>
#include<unordered_set>
#include<boost/circular_buffer.hpp>
#include"TcpConnection.h"

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;



struct Entry
{
	explicit Entry(const WeakTcpConnectionPtr& weakConn)
		: weakConn_(weakConn)
	{

	}

	~Entry()
	{
		TcpConnectionPtr conn = weakConn_.lock();
		if(conn)
		{
			//一旦引用计数减为0，会调用ENTRY的析构函数，断开连接
			conn->shutdown();
		}
	}

	WeakTcpConnectionPtr weakConn_;
};

typedef std::weak_ptr<Entry> WeakEntryPtr;
typedef std::shared_ptr<Entry> EntryPtr;

#endif
