#include "stdafx.h"
#include "MyThread.h"

unsigned int MyThread::activeThreadCount_ = 0;

unsigned int MyThread::threadNameLabel_ = 0;

HANDLE MyThread::hMutex = NULL;

MyThread::MyThread(std::function<unsigned int()> func)
	: hThread_(nullptr),
	threadId_(0),
	threadFunc_(func)
{
	++threadNameLabel_;
	const size_t bufSize = 32;
	threadName_ = new char[bufSize];
	snprintf(threadName_, bufSize, "thread-%u", threadNameLabel_);
}

MyThread::~MyThread()
{
	CleanUp();

	if (activeThreadCount_ == 0 && hMutex != NULL) {

		CloseHandle(hMutex);
		hMutex = NULL;
	}
}

unsigned int MyThread::ActiveCount()
{
	return activeThreadCount_;
}

void MyThread::Start()
{
	if (hMutex == NULL)
	{
		hMutex = CreateMutex(NULL, FALSE, NULL);
		if (hMutex == NULL)
		{
			DEBUG_BREAK();
			return;
		}
	}

	hThread_ = (HANDLE)_beginthreadex(NULL, 0, MyThread::ThreadProc, this, 0, &threadId_);

	if (hThread_ == NULL)
	{
		return;
	}

	MyThread::Lock();
	++activeThreadCount_;
	MyThread::UnLock();
}

void MyThread::Join()
{
	if (hThread_ != NULL) {
		WaitForSingleObject(hThread_, INFINITE);
		CloseHandle(hThread_);
		hThread_ = NULL;
	}
}

void MyThread::Lock()
{
	if (hMutex != NULL) {
		WaitForSingleObject(hMutex, INFINITE);
	}
}

void MyThread::UnLock()
{
	if (hMutex != NULL) {
		ReleaseMutex(hMutex);
	}
}

char* MyThread::GetThreadName() const
{
	return threadName_;
}

HANDLE MyThread::GetHandle() const
{
	return hThread_;
}


void MyThread::CleanUp()
{
	if (threadName_ != nullptr)
	{
		delete[] threadName_;
		threadName_ = nullptr;
	}

	if (hThread_ != NULL)
	{
		CloseHandle(hThread_);
		hThread_ = NULL;
	}
}

unsigned __stdcall MyThread::ThreadProc(void* param)
{
	unsigned ret = 0;
	MyThread* pThisThread = static_cast<MyThread*>(param);
	if (pThisThread != nullptr)
	{
		ret = pThisThread->threadFunc_();
	}
	else
	{
		DEBUG_BREAK();
		ret = 0;
	}

	pThisThread->Lock();
	--activeThreadCount_;
	pThisThread->UnLock();

	_endthreadex(0);

	return ret;
}