#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include "MyThread.h"

std::vector<HANDLE> MyThread::activeThread_;

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

    if (activeThread_.size() == 0 && hMutex != NULL) {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}

size_t MyThread::ActiveCount()
{
    return activeThread_.size();
}

void MyThread::Start()
{
    hThread_ = (HANDLE)_beginthreadex(NULL, 0, MyThread::ThreadProc, this, 0, &threadId_);
    if (hThread_ == NULL)
    {
        return;
    }

    MyThread::Lock();
    activeThread_.push_back(hThread_);
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
    auto it = std::find(activeThread_.begin(), activeThread_.end(), pThisThread->GetHandle());
    if (it != activeThread_.end()) 
    {
        activeThread_.erase(it);
    }
    pThisThread->UnLock();

    _endthreadex(0);

    return ret;
}