#ifndef UV_THREAD_HPP
#define UV_THREAD_HPP

#define HAVE_PTHREADS

#include <uv.h>
#include "Mutex.hpp"

typedef void (*uv_thread_cb)(void* arg);

class Thread {
public:
	class Runnable {
	public:
		virtual ~Runnable() = default;
	private:

	public:
		virtual void run() = 0;
	};
public:
	Thread(Thread::Runnable *runnable);
	~Thread();
	int GetId();
	int Join();
	int RequestExit();
	int RequestExitAndWait();
	bool IsQuit();
friend class Runnable;
private:
	uv_thread_t thread;
	volatile bool           exitPending;
	Mutex mutex {false};
private:

	// A Thread cannot be copied
	Thread(const Thread&);
	Thread& operator =(const Thread&);
};

#if defined(HAVE_PTHREADS)
void ThreadRun(void *arg) {
	Thread::Runnable *runnable = (Thread::Runnable *)arg;
	runnable->run();
}

Thread::Thread(Thread::Runnable *runnable) : exitPending(false) {
	uv_thread_create(&thread, ThreadRun, (void*)runnable);
}

Thread::~Thread() {

}


int Thread::GetId() {
	return (int)uv_thread_self();
}

int Thread::Join() {
	return uv_thread_join(&thread);
}

int Thread::RequestExit() {
	AutoMutex lock(mutex);
	exitPending = true;
	return 0;
}

int Thread::RequestExitAndWait() {
	AutoMutex lock(mutex);
	exitPending = true;
	Join();
	return 0;
}

bool Thread::IsQuit() {
	AutoMutex lock(mutex);
	return (exitPending == true);
}

#endif//HAVE_PTHREADS

#endif//UV_THREAD_HPP

