#ifndef UV_MUTEXT_HPP
#define UV_MUTEXT_HPP

#define HAVE_PTHREADS
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#if defined(HAVE_PTHREADS)
# include <pthread.h>
#endif
#include <uv.h>

class UVCondition;

/*
 * Simple mutex class.  The implementation is system-dependent.
 *
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class UVMutex {
public:
	UVMutex(bool recursive);
	~UVMutex();

	// lock or unlock the mutex
	void lock();
	void unlock();

	// lock if possible; returns 0 on success, error otherwise
	int tryLock();

	// Manages the mutex automatically. It'll be locked when Autolock is
	// constructed and released when Autolock goes out of scope.
	class Autolock {
	public:
		inline Autolock(UVMutex &mutex) :
				lock(mutex) {
			lock.lock();
		}
		inline Autolock(UVMutex *mutex) :
				lock(*mutex) {
			lock.lock();
		}
		inline ~Autolock() {
			lock.unlock();
		}
	private:
		UVMutex &lock;
	};

private:
	friend class AVCondition;
	// A mutex cannot be copied
	UVMutex(const UVMutex&);
	UVMutex& operator =(const UVMutex&);

#if defined(HAVE_PTHREADS)
	uv_mutex_t mutex;
#else
    void    _init();
    void*   mState;
#endif
};

// ---------------------------------------------------------------------------

#if defined(HAVE_PTHREADS)

inline UVMutex::UVMutex(bool recursive) {
	if (recursive == true) {
		uv_mutex_init_recursive(&mutex);
	} else {
		uv_mutex_init(&mutex);
	}

}

inline UVMutex::~UVMutex() {
	uv_mutex_destroy(&mutex);
}
inline void UVMutex::lock() {
	uv_mutex_lock(&mutex);
}
inline void UVMutex::unlock() {
	uv_mutex_unlock(&mutex);
}
inline int UVMutex::tryLock() {
	return uv_mutex_trylock(&mutex);
}

#endif // HAVE_PTHREADS

// ---------------------------------------------------------------------------

/*
 * Automatic mutex.  Declare one of these at the top of a function.
 * When the function returns, it will go out of scope, and release the
 * mutex.
 */

typedef UVMutex::Autolock AutoMutex;

#endif // UV_MUTEXT_HPP

/*********************************************************************************************************
 /*
 *  在线程实际运行过程中，我们经常需要多个线程保持同步。这时可以用互斥锁来完成任务；互斥锁的使用过程中，主要有pthread_mutex_init，pthread_mutex_destory，pthread_mutex_lock，pthread_mutex_unlock这几个函数以完成锁的初始化，锁的销毁，上锁和释放锁操作。



 一，锁的创建

     锁可以被动态或静态创建，可以用宏PTHREAD_MUTEX_INITIALIZER来静态的初始化锁，采用这种方式比较容易理解，互斥锁是pthread_mutex_t的结构体，而这个宏是一个结构常量，如下可以完成静态的初始化锁：

     pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

     另外锁可以用pthread_mutex_init函数动态的创建，函数原型如下：

     int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t * attr)

 二，锁的属性

     互斥锁属性可以由pthread_mutexattr_init(pthread_mutexattr_t *mattr);来初始化，然后可以调用其他的属性设置方法来设置其属性；

     互斥锁的范围：可以指定是该进程与其他进程的同步还是同一进程内不同的线程之间的同步。可以设置为PTHREAD_PROCESS_SHARE和PTHREAD_PROCESS_PRIVATE。默认是后者，表示进程内使用锁。可以使用int pthread_mutexattr_setpshared(pthread_mutexattr_t *mattr, int pshared)

 pthread_mutexattr_getshared(pthread_mutexattr_t *mattr,int *pshared)

 用来设置与获取锁的范围；

     互斥锁的类型：有以下几个取值空间：

 PTHREAD_MUTEX_TIMED_NP，这是缺省值，也就是普通锁。当一个线程加锁以后，其余请求锁的线程将形成一个等待队列，并在解锁后按优先级获得锁。这种锁策略保证了资源分配的公平性。

 PTHREAD_MUTEX_RECURSIVE_NP，嵌套锁，允许同一个线程对同一个锁成功获得多次，并通过多次unlock解锁。如果是不同线程请求，则在加锁线程解锁时重新竞争。

 PTHREAD_MUTEX_ERRORCHECK_NP，检错锁，如果同一个线程请求同一个锁，则返回EDEADLK，否则与PTHREAD_MUTEX_TIMED_NP类型动作相同。这样就保证当不允许多次加锁时不会出现最简单情况下的死锁。

 PTHREAD_MUTEX_ADAPTIVE_NP，适应锁，动作最简单的锁类型，仅等待解锁后重新竞争。

 可以用
 pthread_mutexattr_settype(pthread_mutexattr_t *attr , int type)
 pthread_mutexattr_gettype(pthread_mutexattr_t *attr , int *type)

 获取或设置锁的类型。

 三，锁的释放

     调用pthread_mutex_destory之后，可以释放锁占用的资源，但这有一个前提上锁当前是没有被锁的状态。

 四，锁操作

     对锁的操作主要包括加锁 pthread_mutex_lock()、解锁pthread_mutex_unlock()和测试加锁 pthread_mutex_trylock()三个。

 int pthread_mutex_lock(pthread_mutex_t *mutex)

 int pthread_mutex_unlock(pthread_mutex_t *mutex)

 int pthread_mutex_trylock(pthread_mutex_t *mutex)

 pthread_mutex_trylock()语义与pthread_mutex_lock()类似，不同的是在锁已经被占据时返回EBUSY而不是挂起等待

 原文链接：https://blog.csdn.net/zmxiangde_88/article/details/7998458


 */
