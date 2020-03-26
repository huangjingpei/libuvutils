#ifndef UV_MUTEXT_HPP
#define UV_MUTEXT_HPP
#define HAVE_PTHREADS

#include <uv.h>

typedef void (*uv_thread_cb)(void* arg);

class UVThread {
public:
	class Runnable {
	public:
		virtual ~Runnable() = default;

	public:
		virtual void run() = 0;
	};
public:
	UVThread(UVThread::Runnable *runnable, void *arg);
	~UVThread();
	int GetId();
	int Join();


private:
	uv_thread_t thread;

private:

	// A UVThread cannot be copied
	UVThread(const UVThread&);
	UVThread& operator =(const UVThread&);
};



UV_EXTERN int uv_thread_create(uv_thread_t* tid, uv_thread_cb entry, void* arg);
UV_EXTERN uv_thread_t uv_thread_self(void);
UV_EXTERN int uv_thread_join(uv_thread_t *tid);
UV_EXTERN int uv_thread_equal(const uv_thread_t* t1, const uv_thread_t* t2);

#if defined(HAVE_PTHREADS)
void ThreadRun(void *arg) {
	UVThread::Runnable *runnable = (UVThread::Runnable *)arg;
	runnable->run();
}

UVThread::UVThread(UVThread::Runnable *runnable, void *arg) {
	uv_thread_create(&thread, ThreadRun, arg);
}

UVThread::~UVThread() {

}


int UVThread::GetId() {
	return (int)uv_thread_self();
}

int UVThread::Join() {
	return uv_thread_join(&thread);
}


#endif//HAVE_PTHREADS

#endif//UV_MUTEXT_HPP

