#define UV_CLASS "Timer"
// #define UV_LOG_DEV_LEVEL 3

#include "Timer.hpp"
#include "DepLibUV.hpp"
#include "Logger.hpp"
#include "LibUVErrors.hpp"
#include <uv.h>

/* Static methods for UV callbacks. */

inline static void onTimer(uv_timer_t *handle) {
	static_cast<Timer*>(handle->data)->OnUvTimer();
}

inline static void onClose(uv_handle_t *handle) {
	delete handle;
}

/* Instance methods. */

Timer::Timer(Listener *listener) :
		listener(listener) {


	this->uvHandle = new uv_timer_t;
	this->uvHandle->data = static_cast<void*>(this);

	int err = uv_timer_init(DepLibUV::GetLoop(), this->uvHandle);

	if (err != 0) {
		delete this->uvHandle;
		this->uvHandle = nullptr;

		UV_THROW_ERROR("uv_timer_init() failed: %s", uv_strerror(err));
	}
}

Timer::~Timer() {


	if (!this->closed)
		Close();
}

void Timer::Close() {


	if (this->closed)
		return;

	this->closed = true;

	uv_close(reinterpret_cast<uv_handle_t*>(this->uvHandle),
			static_cast<uv_close_cb>(onClose));
}

void Timer::Start(uint64_t timeout, uint64_t repeat) {


	if (this->closed)
		UV_THROW_ERROR("closed");

	this->timeout = timeout;
	this->repeat = repeat;

	if (uv_is_active(reinterpret_cast<uv_handle_t*>(this->uvHandle)) != 0)
		Stop();

	int err = uv_timer_start(this->uvHandle, static_cast<uv_timer_cb>(onTimer),
			timeout, repeat);

	if (err != 0)
		UV_THROW_ERROR("uv_timer_start() failed: %s", uv_strerror(err));
}

void Timer::Stop() {


	if (this->closed)
		UV_THROW_ERROR("closed");

	int err = uv_timer_stop(this->uvHandle);

	if (err != 0)
		UV_THROW_ERROR("uv_timer_stop() failed: %s", uv_strerror(err));
}

void Timer::Reset() {


	if (this->closed)
		UV_THROW_ERROR("closed");

	if (uv_is_active(reinterpret_cast<uv_handle_t*>(this->uvHandle)) == 0)
		return;

	if (this->repeat == 0u)
		return;

	int err = uv_timer_start(this->uvHandle, static_cast<uv_timer_cb>(onTimer),
			this->repeat, this->repeat);

	if (err != 0)
		UV_THROW_ERROR("uv_timer_start() failed: %s", uv_strerror(err));
}

void Timer::Restart() {


	if (this->closed)
		UV_THROW_ERROR("closed");

	if (uv_is_active(reinterpret_cast<uv_handle_t*>(this->uvHandle)) != 0)
		Stop();

	int err = uv_timer_start(this->uvHandle, static_cast<uv_timer_cb>(onTimer),
			this->timeout, this->repeat);

	if (err != 0)
		UV_THROW_ERROR("uv_timer_start() failed: %s", uv_strerror(err));
}

inline void Timer::OnUvTimer() {


	// Notify the listener.
	this->listener->OnTimer(this);
}
