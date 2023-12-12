#pragma once
#include <atomic>
#include <list>
#include "Thread.h"

#pragma warning(disable:4407)

template<class T>
class CQueue
{
public:
	typedef struct IOCP_param {
		int nOperator;
		HANDLE hEvent;
		T Data;
		IOCP_param(int op, const T& sData, HANDLE hevent = NULL)
		{
			nOperator = op;
			Data = sData;
			hEvent = hevent;
		}
		IOCP_param() {
			nOperator = -1;
		}
	}PPARAM;

	enum
	{
		IOCP_LIST_PUSH,
		IOCP_LIST_POP,
		IOCP_LIST_CLEAR,
		IOCP_LIST_SIZE
	};

	CQueue()
	{
		m_lock = FALSE;
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_Thread = INVALID_HANDLE_VALUE;
		if (m_hIOCP != NULL)
		{
			m_Thread = (HANDLE)_beginthread(&CQueue<T>::ThreadEntr, 0, this);
		}

	}

	virtual ~CQueue() {
		if (m_lock)return;
		m_lock = TRUE;
		PostQueuedCompletionStatus(m_hIOCP, 0, NULL, NULL);
		WaitForSingleObject(m_Thread, INFINITE);
		if (m_hIOCP!=NULL)
		{
			HANDLE hTemp = m_hIOCP;
			m_hIOCP = NULL;
			CloseHandle(hTemp);
		}
	}

	virtual void DealParam(PPARAM* param)
	{
		switch (param->nOperator)
		{
		case IOCP_LIST_PUSH:
			m_lstData.push_back(param->Data);
			delete param;
			break;
		case IOCP_LIST_POP:
			if (m_lstData.size() > 0)
			{
				param->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (param->hEvent != NULL)SetEvent(param->hEvent);
			break;
		case IOCP_LIST_SIZE:
			param->nOperator = (int)m_lstData.size();
			if (param->hEvent != NULL)SetEvent(param->hEvent);
			break;
		case IOCP_LIST_CLEAR:
			m_lstData.clear();
			delete param;
		default:
			OutputDebugString("unknown operator\n");
			break;
		}
	}

	virtual BOOL  PopFront(T& data) {
		if (m_lock)return FALSE;
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		PPARAM pParam(IOCP_LIST_POP, data, hEvent);
		BOOL ret = PostQueuedCompletionStatus(m_hIOCP, sizeof(PPARAM),(ULONG_PTR)&pParam,NULL);
		if (ret == FALSE) {
			CloseHandle(hEvent);
			return FALSE;
		}
		ret = (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0);
		if (ret)data = pParam.Data;
		return ret;
	}

	BOOL PushBack(const T& data)
	{
		if (m_lock)return FALSE;
		PPARAM* Pparam = new PPARAM(IOCP_LIST_PUSH, data);
		BOOL ret = PostQueuedCompletionStatus(m_hIOCP, sizeof(PPARAM), (ULONG_PTR)Pparam, NULL);
		if (ret == FALSE)delete Pparam;
		return ret;
	}

	int Size()
	{
		if (m_lock)return -1;
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		PPARAM pParam(IOCP_LIST_SIZE, T(),hEvent);
		BOOL ret = PostQueuedCompletionStatus(m_hIOCP, sizeof(PPARAM), (ULONG_PTR)&pParam, NULL);
		if (ret == FALSE) {
			CloseHandle(hEvent);
			return FALSE;
		}
		ret = (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0);
		if (ret)return pParam.nOperator;
		return -1;
	}
	BOOL Clear()
	{
		if (m_lock)return FALSE;
		PPARAM* Pparam = new PPARAM(IOCP_LIST_CLEAR, T());
		BOOL ret = PostQueuedCompletionStatus(m_hIOCP, sizeof(PPARAM), (ULONG_PTR)Pparam, NULL);
		if (ret == FALSE)delete Pparam;
		return ret;
	}

protected:
	static void ThreadEntr(void* arg)
	{
		CQueue<T>* thiz = (CQueue<T>*)arg;
		thiz->ThreadMain();
		_endthread();
	}
	void ThreadMain()
	{
		PPARAM* param = NULL;
		DWORD dwTransferred = 0;
		ULONG_PTR dwCompletionKey = 0;
		LPOVERLAPPED lpOverlapped = NULL;
		while (GetQueuedCompletionStatus(m_hIOCP, &dwTransferred, &dwCompletionKey, &lpOverlapped, INFINITE))
		{
			if ((dwTransferred == 0) || (dwCompletionKey == NULL))
			{
				break;
			}
			param = (PPARAM*)dwCompletionKey;
			DealParam(param);
		}
		while (GetQueuedCompletionStatus(m_hIOCP, &dwTransferred, &dwCompletionKey, &lpOverlapped,0))
		{
			if ((dwTransferred == 0) || (dwCompletionKey == NULL))
			{
				printf("检查是否有残余数据\n");
				continue;
			}
			param = (PPARAM*)dwCompletionKey;
			DealParam(param);
		}
		HANDLE hTemp = m_hIOCP;
		m_hIOCP = NULL;
		CloseHandle(hTemp);
	}
protected:
	std::list<T> m_lstData;
	HANDLE m_hIOCP;
	HANDLE m_Thread;
	std::atomic<BOOL>m_lock;
};




template<class T>
class SendQueue:public ThreadFuncBase,public CQueue<T>
{
public:
	typedef int (ThreadFuncBase::* Callback)(T& data);
	SendQueue(ThreadFuncBase* obj, Callback callback):CQueue<T>(), m_base(obj), m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdataWorker(ThreadWorker(this,(FUNCTYPE)&SendQueue<T>::ThreadTick));
	}
	virtual ~SendQueue() {
		m_thread.Stop();
		m_base = NULL;
		m_callback = NULL;
	
	}
protected:
	virtual BOOL  PopFront(T& data) { return FALSE; }
	BOOL PopFront()
	{
		typename CQueue<T>::PPARAM* pParam=new typename CQueue<T>::PPARAM(CQueue<T>::IOCP_LIST_POP,T());
		if (CQueue<T>::m_lock)
		{
			delete pParam;
			return FALSE;
		}
		BOOL ret = PostQueuedCompletionStatus(CQueue<T>::m_hIOCP, sizeof(CQueue<T>::PPARAM),(ULONG_PTR)&pParam, NULL);
		if (ret == FALSE)
		{
			delete pParam;
			return FALSE;
		}
		return ret;
	}

	int ThreadTick()
	{
		if ((WaitForSingleObject(CQueue<T>::m_Thread, 0) != WAIT_TIMEOUT))return 0;
		if (CQueue<T>::m_lstData.size()>0)
		{
			PopFront();
		}
		Sleep(1);
		return 0;
	}

	virtual void DealParam(typename CQueue<T>::PPARAM* param)
	{
		switch (param->nOperator)
		{
		case CQueue<T>::IOCP_LIST_PUSH:
			CQueue<T>::m_lstData.push_back(param->Data);
			delete param;
			break;
		case CQueue<T>::IOCP_LIST_POP:
			if (CQueue<T>::m_lstData.size() > 0)
			{
				param->Data = CQueue<T>::m_lstData.front();
				if((m_base->*m_callback)(param->Data)==0)
					CQueue<T>::m_lstData.pop_front();
			}
			delete param;
			break;
		case CQueue<T>::IOCP_LIST_SIZE:
			param->nOperator = (int)CQueue<T>::m_lstData.size();
			if (param->hEvent != NULL)SetEvent(param->hEvent);
			break;
		case CQueue<T>::IOCP_LIST_CLEAR:
			CQueue<T>::m_lstData.clear();
			delete param;
			break;
		default:
			OutputDebugString("unknown operator\n");
			break;
		}
	}

 private:
	 ThreadFuncBase* m_base;
	 Callback m_callback;
	 CThread m_thread;
};
typedef SendQueue<std::vector<char>>::Callback SENDCALLBACK;