#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include "MyThread.h"

unsigned int MyThread::activeCount = 0;

unsigned int MyThread::threadNum = 0;

HANDLE MyThread::hMutex = NULL;

MyThread::MyThread(std::function<unsigned int()> func)
    : hThread_(nullptr),
    threadId_(0),
    threadFunc_(func)
{
    ++threadNum;
    const size_t bufSize = 32;
    threadName_ = new char[bufSize];
    snprintf(threadName_, bufSize, "thread-%u", threadNum);

    if (hMutex == NULL)
    {
        hMutex = CreateMutex(NULL, FALSE, NULL);
        if (!hMutex)
        {
            return;
        }
    }
}

MyThread::~MyThread()
{
    CleanUp();

    if (activeCount == 0 && hMutex != NULL) {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}

unsigned int MyThread::ActiveCount()
{
    return activeCount;
}

void MyThread::Start()
{
    hThread_ = (HANDLE)_beginthreadex(NULL, 0, MyThread::ThreadProc, this, 0, &threadId_);
    if (hThread_ == NULL)
    {
        return;
    }

    MyThread::Lock();
    ++MyThread::activeCount;
    MyThread::Unlock();
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

void MyThread::Unlock()
{
    if (hMutex != NULL) {
        ReleaseMutex(hMutex);
    }
}

char* MyThread::GetThreadName() const
{
    return threadName_;
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
    --MyThread::activeCount;
    pThisThread->Unlock();

    _endthreadex(0);

    return ret;
}