#pragma once


class MyThread {
public:
    MyThread(std::function<unsigned int()> func);

    virtual ~MyThread();

    static unsigned int ActiveCount();

    void Start();

    void Join();

    void Lock();

    void Unlock();

    char* GetThreadName() const;

protected:
    //virtual unsigned Run() = 0;
        
    void CleanUp();

private:
    static unsigned __stdcall ThreadProc(void* param);

private:
    static HANDLE hMutex;

    static unsigned int activeCount;

    static unsigned int threadNum;

    HANDLE hThread_;

    unsigned int threadId_;

    char* threadName_;

    std::function<unsigned int()> threadFunc_;
};
