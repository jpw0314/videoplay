#pragma once
#include <atomic>
#include <vector>
#include <mutex>
#include "Tool.h"

class ThreadFuncBase{};
typedef int (ThreadFuncBase::*FUNCTYPE) ();

class ThreadWorker { 
public:
	ThreadWorker():thiz{NULL},func{NULL}{}
	ThreadWorker(void* obj, FUNCTYPE f) :thiz{ (ThreadFuncBase*)obj },func{f}{}
	ThreadWorker(const ThreadWorker& woker)
	{
		thiz = woker.thiz;
		func = woker.func;
	}
	ThreadWorker& operator=(const ThreadWorker& woker){
	if (this!=&woker)
	{
		thiz = woker.thiz;
		func = woker.func;
	}
	return *this;
	}

	int operator()()
	{
		if (IsValid())
		{
			return (thiz->*func)();
		}
		return -1;
	}

	BOOL IsValid() const{
		return (thiz != NULL) && (func != NULL);
	}

private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};

class CThread
{

public:
	CThread()
	{
		m_Thread = NULL;
		m_hStatus = FALSE;
	}
	~CThread()
	{
		Stop();
	}
	//true 表示线程启动成功 false 表示失败
	BOOL Start()
	{
		m_hStatus = TRUE;
		m_Thread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
		if (!IsValid())
		{
			m_hStatus = FALSE; 
		}
		return m_hStatus; 
	}
	BOOL Stop()
	{
		if (m_hStatus == FALSE)return TRUE;
		m_hStatus = FALSE;
		bool ret= (WaitForSingleObject(m_Thread, 1000) == WAIT_OBJECT_0);
// 		if (ret==WAIT_TIMEOUT)
// 		{
// 			TerminateThread(m_Thread,-1);
// 		}
		UpdataWorker();
		return ret;
	}
	//返回TRUE表示有效,返回false表示线程异常或终止
	BOOL IsValid()
	{
		if ((m_Thread == NULL) ||( m_Thread == INVALID_HANDLE_VALUE))return FALSE;
		return (WaitForSingleObject(m_Thread, 0) == WAIT_TIMEOUT);
	}

	void UpdataWorker(const ThreadWorker& woker=ThreadWorker())
	{
		if (m_woker.load()!=NULL)
		{
			ThreadWorker* pWorker = m_woker.load();
			m_woker.store(NULL);
			delete pWorker;
		}
		if (m_woker.load() == &woker)return;
		if (!woker.IsValid())
		{
			m_woker.store(NULL);
			return;
		}
		m_woker.store(new ThreadWorker(woker));
	}

	BOOL IsIdle()
	{
		if (m_woker.load() == NULL)return TRUE;
		return !m_woker.load()->IsValid();
	}
private:
   void  ThreadWork()
	{
		while (m_hStatus)
		{
			if (m_woker.load()==NULL)
			{
				Sleep(1);
				continue;
			}
			ThreadWorker woker = *m_woker.load();
			if (woker.IsValid())
			{
				if (WaitForSingleObject(m_Thread,0)==WAIT_TIMEOUT)
				{
					int ret = woker();
					if (ret != 0)
					{
						TRACE("thread found warning ret =%d\n",ret);
						break;
					}
					else if (ret < 0)
					{
						ThreadWorker* pWorker = m_woker.load();
						m_woker.store(NULL);
						delete pWorker;
					}
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}
	static void ThreadEntry(void* arg)
	{
		CThread* thiz = (CThread*)arg;
		if (thiz)
		{
			thiz->ThreadWork();
		}
		_endthread();
	}
private:
	HANDLE m_Thread;
	BOOL m_hStatus;//线程状态 FALSE表示线程将要关闭 TRUE表示线程正在运行
	std::atomic<ThreadWorker*> m_woker;
};

class ThreadPool
{
public:
	ThreadPool(size_t size)
	{
		m_threadpool.resize(size);
		for (size_t i=0;i<size;i++)
		{
			m_threadpool[i] = new CThread();
		}
	}
	ThreadPool(){}
	~ThreadPool(){
		Stop();
		for (size_t i = 0; i < m_threadpool.size(); i++)
		{
			delete m_threadpool[i];
			m_threadpool[i] = NULL;
		}
		m_threadpool.clear(); 
	}
	BOOL Invoke()
	{
		BOOL ret = TRUE;
		for (size_t i=0;i<m_threadpool.size();i++)
		{
			if (m_threadpool[i]->Start() == FALSE)
			{
				ret = FALSE;
				break;
			}
			if (ret == FALSE)
			{
				for (size_t i = 0; i < m_threadpool.size(); i++)
				{
					m_threadpool[i]->Stop();
				}
			}
		}
		return ret;
	}
	void Stop()
	{
		for (size_t i = 0; i < m_threadpool.size(); i++)
		{
			m_threadpool[i]->Stop();
		}
	}
	//返回-1 表示分配失败
	int DispatchWorker(const ThreadWorker& worker )
	{
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threadpool.size(); i++)
		{
			if (m_threadpool[i]->IsIdle())
			{
				m_threadpool[i]->UpdataWorker(worker);
				index = (int)i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}
	//检查线程有效性
	BOOL CheckThreadValid(size_t index)
	{
		if (index < m_threadpool.size())
		{
			return m_threadpool[index]->IsValid();
		}
		return FALSE;
	}
private:
	std::mutex m_lock;
	std::vector<CThread*> m_threadpool;
};
