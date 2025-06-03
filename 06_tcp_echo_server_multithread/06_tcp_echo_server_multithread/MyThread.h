#pragma once

class MyThread {
public:
    MyThread(std::function<unsigned int()> func);

    virtual ~MyThread();

    static unsigned int ActiveCount();

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

    static unsigned int activeThreadCount_;

    static unsigned int threadNameLabel_;

    HANDLE hThread_;

    unsigned int threadId_;

    char* threadName_;

    std::function<unsigned int()> threadFunc_;
};

