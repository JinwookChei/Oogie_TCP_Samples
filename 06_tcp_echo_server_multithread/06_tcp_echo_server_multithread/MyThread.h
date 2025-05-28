#pragma once
#include <vector>

class MyThread {
public:
    MyThread(std::function<unsigned int()> func);

    virtual ~MyThread();

    static size_t ActiveCount();

    void Start();

    void Join();

    void Lock();

    void UnLock();

    char* GetThreadName() const;

    HANDLE GetHandle() const;

protected:

    void CleanUp();

private:
    static unsigned __stdcall ThreadProc(void* param);

private:
    static HANDLE hMutex;

    static std::vector<HANDLE> activeThread_;

    static unsigned int threadNum;

    HANDLE hThread_;

    unsigned int threadId_;

    char* threadName_;

    std::function<unsigned int()> threadFunc_;
};
