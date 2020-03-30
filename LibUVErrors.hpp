#ifndef LIBUV_ERRORS_HPP
#define LIBUV_ERRORS_HPP


#include <cstdio> // std::snprintf()
#include <stdexcept>
#include "Logger.hpp"

class LibUVError : public std::runtime_error
{
public:
	explicit LibUVError(const char* description);
};

/* Inline methods. */

inline LibUVError::LibUVError(const char* description) : std::runtime_error(description)
{
}

class LibUVTypeError : public LibUVError
{
public:
	explicit LibUVTypeError(const char* description);
};

/* Inline methods. */

inline LibUVTypeError::LibUVTypeError(const char* description) : LibUVError(description)
{
}

// clang-format off
#define UV_THROW_ERROR(desc, ...) \
	do \
	{ \
		UV_ERROR("throwing LibUVError: " desc, ##__VA_ARGS__); \
		\
		static char buffer[2000]; \
		\
		std::snprintf(buffer, 2000, desc, ##__VA_ARGS__); \
		throw LibUVError(buffer); \
	} while (false)

#define UV_THROW_ERROR_STD(desc, ...) \
	do \
	{ \
		UV_ERROR_STD("throwing LibUVError: " desc, ##__VA_ARGS__); \
		\
		static char buffer[2000]; \
		\
		std::snprintf(buffer, 2000, desc, ##__VA_ARGS__); \
		throw LibUVError(buffer); \
	} while (false)

#define UV_THROW_TYPE_ERROR(desc, ...) \
	do \
	{ \
		UV_ERROR("throwing LibUVTypeError: " desc, ##__VA_ARGS__); \
		\
		static char buffer[2000]; \
		\
		std::snprintf(buffer, 2000, desc, ##__VA_ARGS__); \
		throw LibUVTypeError(buffer); \
	} while (false)

#define UV_THROW_TYPE_ERROR_STD(desc, ...) \
	do \
	{ \
		UV_ERROR_STD("throwing LibUVTypeError: " desc, ##__VA_ARGS__); \
		\
		static char buffer[2000]; \
		\
		std::snprintf(buffer, 2000, desc, ##__VA_ARGS__); \
		throw LibUVTypeError(buffer); \
	} while (false)
// clang-format on

#endif //LIBUV_ERRORS_HPP
