#include "stdafx.h"
#include "MyThread.h"

unsigned int MyThread::threadCount = 0;

MyThread::MyThread(std::function<void()> job)
	:threadName_(0),
	threadId_(0),
	handle_(0),
	job_(job)
{
	_CreateMutex();
}

MyThread::~MyThread()
{
	// TODO : 만약 마지막 쓰레드면 뮤텍스도 삭제해야함.

	Shutdown();
}

bool MyThread::_CreateMutex()
{
	if (hMutex != nullptr)
	{
		return false;
	}

	MyThread::hMutex = CreateMutex(NULL, FALSE, NULL);
	if (MyThread::hMutex == NULL)
	{
		DEBUG_BREAK();
		return false;
	}

	return true;
}

void MyThread::Start()
{
	unsigned int threadId = 0;	
	handle_ = (HANDLE)_beginthreadex(nullptr, 0, MyThread::ThreadFunc, this, 0, &threadId_);
	if (handle_ == 0)
	{
		DEBUG_BREAK();
		return;
	}
}

void MyThread::Shutdown()
{
	WaitForSingleObject(handle_, INFINITE);

	CloseHandle(hMutex);
	CloseHandle(handle_);
}

unsigned int MyThread::GetThreadName() const
{
	return threadName_;
}

unsigned int __stdcall MyThread::ThreadFunc(void* myThreadPointer)
{
	MyThread* myThreadPtr = (MyThread*)myThreadPointer;
	if (myThreadPtr == nullptr)
	{
		_endthreadex(-1);
		return -1;
	}
	
	if (myThreadPtr->job_ == false)
	{
		_endthreadex(-1);
		return -1;
	}

	// MUTEX
	WaitForSingleObject(myThreadPtr->hMutex, INFINITE);
	threadsInfo.push_back(myThreadPtr);
	myThreadPtr->threadName_ = threadCount;
	++threadCount;
	ReleaseMutex(myThreadPtr->hMutex);
	// MUTEX END

	myThreadPtr->job_();

	return 0;
}


