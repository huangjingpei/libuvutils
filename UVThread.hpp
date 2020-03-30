#ifndef UV_THREAD_HPP
#define UV_THREAD_HPP
#define HAVE_PTHREADS

#include <uv.h>
#include "UVMutex.hpp"

typedef void (*uv_thread_cb)(void* arg);

class UVThread {
public:
	class Runnable {
	public:
		virtual ~Runnable() = default;
	private:

	public:
		virtual void run() = 0;
	};
public:
	UVThread(UVThread::Runnable *runnable);
	~UVThread();
	int GetId();
	int Join();
	int RequestExit();
	int RequestExitAndWait();
	bool IsQuit();
friend class Runnable;
private:
	uv_thread_t thread;
	volatile bool           exitPending;
	UVMutex mutex {false};
private:

	// A UVThread cannot be copied
	UVThread(const UVThread&);
	UVThread& operator =(const UVThread&);
};

#if defined(HAVE_PTHREADS)
void ThreadRun(void *arg) {
	UVThread::Runnable *runnable = (UVThread::Runnable *)arg;
	runnable->run();
}

UVThread::UVThread(UVThread::Runnable *runnable) : exitPending(false) {
	uv_thread_create(&thread, ThreadRun, (void*)runnable);
}

UVThread::~UVThread() {

}


int UVThread::GetId() {
	return (int)uv_thread_self();
}

int UVThread::Join() {
	return uv_thread_join(&thread);
}

int UVThread::RequestExit() {
	AutoMutex lock(mutex);
	exitPending = true;
	return 0;
}

int UVThread::RequestExitAndWait() {
	AutoMutex lock(mutex);
	exitPending = true;
	Join();
	return 0;
}

bool UVThread::IsQuit() {
	AutoMutex lock(mutex);
	return (exitPending == true);
}

#endif//HAVE_PTHREADS

#endif//UV_THREAD_HPP

