#pragma once


class MyThread
{
public:
	MyThread(std::function<void()> job);
	virtual ~MyThread();

	static bool _CreateMutex();

	void Start();

	void Shutdown();

	unsigned int GetThreadName() const;

private:
	static unsigned int __stdcall ThreadFunc(void* myThreadPointer);

private:
	static HANDLE hMutex;

	static std::vector<MyThread*> threadsInfo;

	static unsigned int threadCount;

	unsigned int threadName_;

	unsigned int threadId_;

	HANDLE handle_;
	
	std::function<void()> job_;

	
};

