#define UV_CLASS "DepLibUV"
// #define UV_LOG_DEV_LEVEL 3

#include "DepLibUV.hpp"
#include "Logger.hpp"
#include <cstdlib> // std::abort()
#include "uv.h"

/* Static variables. */

uv_loop_t *DepLibUV::loop { nullptr };

/* Static methods. */

void DepLibUV::ClassInit() {
	// NOTE: Logger depends on this so we cannot log anything here.

	DepLibUV::loop = new uv_loop_t;

	int err = uv_loop_init(DepLibUV::loop);

	if (err != 0) {
		UV_ERROR("libuv initialization failed", loop);
	}
}

void DepLibUV::ClassDestroy() {


	// This should never happen.
	if (DepLibUV::loop != nullptr) {
		uv_loop_close(DepLibUV::loop);
		delete DepLibUV::loop;
	}
}

void DepLibUV::PrintVersion() {
	UV_DEBUG_TAG(UV_CLASS, "libuv version: \"%s\"", uv_version_string());
}

void DepLibUV::RunLoop() {


	// This should never happen.
	UV_ASSERT(DepLibUV::loop != nullptr, "loop unset");

	uv_run(DepLibUV::loop, UV_RUN_DEFAULT);
}
