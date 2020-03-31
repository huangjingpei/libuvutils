/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UV_CONDITION_HPP
#define UV_CONDITION_HPP

#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#if defined(HAVE_PTHREADS)
# include <pthread.h>
#endif


#include "Mutex.hpp"

/*
 * UVCondition variable class.  The implementation is system-dependent.
 *
 * UVCondition variables are paired up with mutexes.  Lock the mutex,
 * call wait(), then either re-wait() if things aren't quite what you want,
 * or unlock the mutex and continue.  All threads calling wait() must
 * use the same mutex for a given UVCondition.
 */
class UVCondition {
public:

    UVCondition();
    ~UVCondition();
    // Wait on the condition variable.  Lock the mutex before calling.
    int wait(Mutex& mutex);
    // same with relative timeout
    int waitRelative(Mutex& mutex, nsecs_t reltime);
    // Signal the condition variable, allowing one thread to continue.
    void signal();
    // Signal the condition variable, allowing all threads to continue.
    void broadcast();

private:
#if defined(HAVE_PTHREADS)
    uv_cond_t cond;
#else
    void*   mState;
#endif
};

// ---------------------------------------------------------------------------

#if defined(HAVE_PTHREADS)

inline UVCondition::UVCondition() {
    uv_cond_init(&cond);
}

inline UVCondition::~UVCondition() {
	uv_cond_destroy(&cond);
}
inline int UVCondition::wait(Mutex& mutex) {
    return -uv_cond_wait(&cond, &mutex.mutex);
}
inline int UVCondition::waitRelative(Mutex& mutex, nsecs_t reltime) {
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE)
    struct timespec ts;
    ts.tv_sec  = reltime/1000000000;
    ts.tv_nsec = reltime%1000000000;
    return -pthread_cond_timedwait_relative_np(&cond, &mutex.mutex, &ts);
#else // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
    struct timespec ts;
#if defined(HAVE_POSIX_CLOCKS)
    clock_gettime(CLOCK_REALTIME, &ts);
#else // HAVE_POSIX_CLOCKS
    // we don't support the clocks here.
    struct timeval t;
    gettimeofday(&t, NULL);
    ts.tv_sec = t.tv_sec;
    ts.tv_nsec= t.tv_usec*1000;
#endif // HAVE_POSIX_CLOCKS
    ts.tv_sec += reltime/1000000000;
    ts.tv_nsec+= reltime%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    return -pthread_cond_timedwait(&cond, &mutex.mutex, &ts);
#endif // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
}
inline void UVCondition::signal() {
    uv_cond_signal(&cond);
}
inline void UVCondition::broadcast() {
    uv_cond_broadcast(&cond);
}

#endif // HAVE_PTHREADS


#endif // UV_CONDITION_HPP
