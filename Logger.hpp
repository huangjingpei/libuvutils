/**
 * Logger facility.
 *
 * This include file defines logging macros for source files (.cpp). Each
 * source file including Logger.hpp MUST define its own UV_CLASS macro. Include
 * files (.hpp) MUST NOT include Logger.hpp.
 *
 * All the logging macros use the same format as printf(). The XXX_STD() version
 * of a macro logs to stdoud/stderr instead of using the Channel instance.
 * However some macros such as UV_ABORT() and UV_ASSERT() always log to stderr.
 *
 * If the macro UV_LOG_STD is defined, all the macros log to stdout/stderr.
 *
 * If the macro UV_LOG_FILE_LINE is defied, all the logging macros print more
 * verbose information, including current file and line.
 *
 * UV_TRACE()
 *
 *   Logs the current method/function if UV_LOG_TRACE macro is defined and the
 *   current log level is "debug".
 *
 * UV_HAS_DEBUG_TAG(tag)
 * UV_HAS_WARN_TAG(tag)
 *
 *   True if the current log level is satisfied and the given tag is enabled.
 *
 * UV_DEBUG_TAG(tag, ...)
 * UV_WARN_TAG(tag, ...)
 *
 *   Logs if the current log level is satisfied and the given tag is enabled.
 *
 *   Example:
 *     UV_WARN_TAG(ice, "ICE failed");
 *
 * UV_DEBUG_2TAGS(tag1, tag2, ...)
 * UV_WARN_2TAGS(tag1, tag2, ...)
 *
 *   Logs if the current log level is satisfied and any of the given two tags
 *   is enabled.
 *
 *   Example:
 *     UV_DEBUG_2TAGS(ice, dtls, "media connection established");
 *
 * UV_DEBUG_DEV(...)
 *
 * 	 Logs if the current source file defines the UV_LOG_DEV_LEVEL macro with
 * 	 value 3.
 *
 * 	 Example:
 * 	   UV_DEBUG_DEV("foo:%" PRIu32, foo);
 *
 * UV_WARN_DEV(...)
 *
 * 	 Logs if the current source file defines the UV_LOG_DEV_LEVEL macro with
 * 	 value >= 2.
 *
 * 	 Example:
 * 	   UV_WARN_DEV("foo:%" PRIu32, foo);
 *
 * UV_DUMP(...)
 *
 * 	 Logs always. Useful for Dump() methods.
 *
 * 	 Example:
 * 	   UV_DUMP("foo");
 *
 * UV_DUMP_DATA(const uint8_t* data, size_t len)
 *
 *   Logs always. Prints the given data in hexadecimal format (Wireshark friendly).
 *
 * UV_ERROR(...)
 *
 *   Logs an error if the current log level is satisfied (or if the current
 *   source file defines the UV_LOG_DEV_LEVEL macro with value >= 1). Must just
 *   be used for internal errors that should not happen.
 *
 * UV_ABORT(...)
 *
 *   Logs the given error to stderr and aborts the process.
 *
 * UV_ASSERT(condition, ...)
 *
 *   If the condition is not satisfied, it calls UV_ABORT().
 */

#ifndef UV_LOGGER_HPP
#define UV_LOGGER_HPP

#include "UnixStreamSocket.hpp"
#include <cstdio>  // std::snprintf(), std::fprintf(), stdout, stderr
#include <cstdlib> // std::abort(), std::getenv()
#include <cstring>

// clang-format off

#define _UV_TAG_ENABLED(tag) Settings::configuration.logTags.tag
#define _UV_TAG_ENABLED_2(tag1, tag2) (Settings::configuration.logTags.tag1 || Settings::configuration.logTags.tag2)

#if !defined(UV_LOG_DEV_LEVEL)
	#define UV_LOG_DEV_LEVEL 0
#elif UV_LOG_DEV_LEVEL < 0 || UV_LOG_DEV_LEVEL > 3
	#error "invalid UV_LOG_DEV_LEVEL macro value"
#endif

// Usage:
//   UV_DEBUG_DEV("Leading text "UV_UINT16_TO_BINARY_PATTERN, UV_UINT16_TO_BINARY(value));
#define UV_UINT16_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define UV_UINT16_TO_BINARY(value) \
	((value & 0x8000) ? '1' : '0'), \
	((value & 0x4000) ? '1' : '0'), \
	((value & 0x2000) ? '1' : '0'), \
	((value & 0x1000) ? '1' : '0'), \
	((value & 0x800) ? '1' : '0'), \
	((value & 0x400) ? '1' : '0'), \
	((value & 0x200) ? '1' : '0'), \
	((value & 0x100) ? '1' : '0'), \
	((value & 0x80) ? '1' : '0'), \
	((value & 0x40) ? '1' : '0'), \
	((value & 0x20) ? '1' : '0'), \
	((value & 0x10) ? '1' : '0'), \
	((value & 0x08) ? '1' : '0'), \
	((value & 0x04) ? '1' : '0'), \
	((value & 0x02) ? '1' : '0'), \
	((value & 0x01) ? '1' : '0')

class Logger
{
public:
	static void ClassInit(UnixStreamSocket* channel);

public:
	static const int64_t pid;
	static UnixStreamSocket* channel;
	static const size_t bufferSize {50000};
	static char buffer[];
};

/* Logging macros. */

#define _UV_LOG_SEPARATOR_CHAR_STD "\n"

#ifdef UV_LOG_FILE_LINE
	#define _UV_LOG_STR "%s:%d | %s::%s()"
	#define _UV_LOG_STR_DESC _UV_LOG_STR " | "
	#define _UV_FILE (std::strchr(__FILE__, '/') ? std::strchr(__FILE__, '/') + 1 : __FILE__)
	#define _UV_LOG_ARG _UV_FILE, __LINE__, UV_CLASS, __FUNCTION__
#else
	#define _UV_LOG_STR "%s::%s()"
	#define _UV_LOG_STR_DESC _UV_LOG_STR " | "
	#define _UV_LOG_ARG UV_CLASS, __FUNCTION__
#endif

#ifdef UV_LOG_TRACE
	#define UV_TRACE() \
		do \
		{ \
			if (Settings::configuration.logLevel == LogLevel::LOG_DEBUG) \
			{ \
				int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "D(trace) " _UV_LOG_STR, _UV_LOG_ARG); \
				Logger::channel->SendLog(Logger::buffer, loggerWritten); \
			} \
		} \
		while (false)

	#define UV_TRACE_STD() \
		do \
		{ \
			if (Settings::configuration.logLevel == LogLevel::LOG_DEBUG) \
			{ \
				std::fprintf(stdout, "(trace) " _UV_LOG_STR _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG); \
				std::fflush(stdout); \
			} \
		} \
		while (false)
#else
	#define UV_TRACE() {}
	#define UV_TRACE_STD() {}
#endif

#define UV_HAS_DEBUG_TAG(tag) \
	(Settings::configuration.logLevel == LogLevel::LOG_DEBUG && _UV_TAG_ENABLED(tag))

#define UV_HAS_WARN_TAG(tag) \
	(Settings::configuration.logLevel >= LogLevel::LOG_WARN && _UV_TAG_ENABLED(tag))

#define UV_DEBUG_TAG(tag, desc, ...) \
	do \
	{ \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "D" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
	} \
	while (false)

#define UV_DEBUG_TAG_STD(tag, desc, ...) \
	do \
	{ \
		{ \
			std::fprintf(stdout, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stdout); \
		} \
	} \
	while (false)

#define UV_WARN_TAG(tag, desc, ...) \
	do \
	{ \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "W" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
	} \
	while (false)

#define UV_WARN_TAG_STD(tag, desc, ...) \
	do \
	{ \
		if (Settings::configuration.logLevel >= LogLevel::LOG_WARN && _UV_TAG_ENABLED(tag)) \
		{ \
			std::fprintf(stderr, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stderr); \
		} \
	} \
	while (false)

#define UV_DEBUG_2TAGS(tag1, tag2, desc, ...) \
	do \
	{ \
		if (Settings::configuration.logLevel == LogLevel::LOG_DEBUG && _UV_TAG_ENABLED_2(tag1, tag2)) \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "D" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
	} \
	while (false)

#define UV_DEBUG_2TAGS_STD(tag1, tag2, desc, ...) \
	do \
	{ \
		if (Settings::configuration.logLevel == LogLevel::LOG_DEBUG && _UV_TAG_ENABLED_2(tag1, tag2)) \
		{ \
			std::fprintf(stdout, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stdout); \
		} \
	} \
	while (false)

#define UV_WARN_2TAGS(tag1, tag2, desc, ...) \
	do \
	{ \
		if (Settings::configuration.logLevel >= LogLevel::LOG_WARN && _UV_TAG_ENABLED_2(tag1, tag2)) \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "W" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
	} \
	while (false)

#define UV_WARN_2TAGS_STD(tag1, tag2, desc, ...) \
	do \
	{ \
		if (Settings::configuration.logLevel >= LogLevel::LOG_WARN && _UV_TAG_ENABLED_2(tag1, tag2)) \
		{ \
			std::fprintf(stderr, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stderr); \
		} \
	} \
	while (false)

#if UV_LOG_DEV_LEVEL == 3
	#define UV_DEBUG_DEV(desc, ...) \
		do \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "D" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
		while (false)

	#define UV_DEBUG_DEV_STD(desc, ...) \
		do \
		{ \
			std::fprintf(stdout, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stdout); \
		} \
		while (false)
#else
	#define UV_DEBUG_DEV(desc, ...) {}
	#define UV_DEBUG_DEV_STD(desc, ...) {}
#endif


#if UV_LOG_DEV_LEVEL >= 2
	#define UV_WARN_DEV(desc, ...) \
		do \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "W" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
		while (false)

	#define UV_WARN_DEV_STD(desc, ...) \
		do \
		{ \
			std::fprintf(stderr, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stderr); \
		} \
		while (false)
#else
	#define UV_WARN_DEV(desc, ...) {}
	#define UV_WARN_DEV_STD(desc, ...) {}
#endif

#define UV_DUMP(desc, ...) \
	do \
	{ \
		int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "X" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
		Logger::channel->SendLog(Logger::buffer, loggerWritten); \
	} \
	while (false)

#define UV_DUMP_STD(desc, ...) \
	do \
	{ \
		std::fprintf(stdout, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
		std::fflush(stdout); \
	} \
	while (false)

#define UV_DUMP_DATA(data, len) \
	do \
	{ \
		int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "X(data) " _UV_LOG_STR, _UV_LOG_ARG); \
		Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		size_t bufferDataLen{ 0 }; \
		for (size_t i{0}; i < len; ++i) \
		{ \
		  if (i % 8 == 0) \
		  { \
		  	if (bufferDataLen != 0) \
		  	{ \
		  		Logger::channel->SendLog(Logger::buffer, bufferDataLen); \
		  		bufferDataLen = 0; \
		  	} \
		    int loggerWritten = std::snprintf(Logger::buffer + bufferDataLen, Logger::bufferSize, "X%06X ", static_cast<unsigned int>(i)); \
		    bufferDataLen += loggerWritten; \
		  } \
		  int loggerWritten = std::snprintf(Logger::buffer + bufferDataLen, Logger::bufferSize, "%02X ", static_cast<unsigned char>(data[i])); \
		  bufferDataLen += loggerWritten; \
		} \
		if (bufferDataLen != 0) \
			Logger::channel->SendLog(Logger::buffer, bufferDataLen); \
	} \
	while (false)

#define UV_DUMP_DATA_STD(data, len) \
	do \
	{ \
		std::fprintf(stdout, "(data) " _UV_LOG_STR _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG); \
		size_t bufferDataLen{ 0 }; \
		for (size_t i{0}; i < len; ++i) \
		{ \
		  if (i % 8 == 0) \
		  { \
		  	if (bufferDataLen != 0) \
		  	{ \
		  		Logger::buffer[bufferDataLen] = '\0'; \
		  		std::fprintf(stdout, "%s", Logger::buffer); \
		  		bufferDataLen = 0; \
		  	} \
		    int loggerWritten = std::snprintf(Logger::buffer + bufferDataLen, Logger::bufferSize, "\n%06X ", static_cast<unsigned int>(i)); \
		    bufferDataLen += loggerWritten; \
		  } \
		  int loggerWritten = std::snprintf(Logger::buffer + bufferDataLen, Logger::bufferSize, "%02X ", static_cast<unsigned char>(data[i])); \
		  bufferDataLen += loggerWritten; \
		} \
		if (bufferDataLen != 0) \
		{ \
			Logger::buffer[bufferDataLen] = '\0'; \
			std::fprintf(stdout, "%s", Logger::buffer); \
		} \
		std::fflush(stdout); \
	} \
	while (false)

#define UV_ERROR(desc, ...) \
	do \
	{ \
		{ \
			int loggerWritten = std::snprintf(Logger::buffer, Logger::bufferSize, "E" _UV_LOG_STR_DESC desc, _UV_LOG_ARG, ##__VA_ARGS__); \
			Logger::channel->SendLog(Logger::buffer, loggerWritten); \
		} \
	} \
	while (false)

#define UV_ERROR_STD(desc, ...) \
	do \
	{ \
		{ \
			std::fprintf(stderr, _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
			std::fflush(stderr); \
		} \
	} \
	while (false)

#define UV_ABORT(desc, ...) \
	do \
	{ \
		std::fprintf(stderr, "(ABORT) " _UV_LOG_STR_DESC desc _UV_LOG_SEPARATOR_CHAR_STD, _UV_LOG_ARG, ##__VA_ARGS__); \
		std::fflush(stderr); \
		std::abort(); \
	} \
	while (false)

#define UV_ASSERT(condition, desc, ...) \
	if (!(condition)) \
	{ \
		UV_ABORT("failed assertion `%s': " desc, #condition, ##__VA_ARGS__); \
	}
#define UV_LOG_STD
#ifdef UV_LOG_STD
	#undef UV_TRACE
	#define UV_TRACE UV_TRACE_STD
	#undef UV_DEBUG_TAG
	#define UV_DEBUG_TAG UV_DEBUG_TAG_STD
	#undef UV_WARN_TAG
	#define UV_WARN_TAG UV_WARN_TAG_STD
	#undef UV_DEBUG_2TAGS
	#define UV_DEBUG_2TAGS UV_DEBUG_2TAGS_STD
	#undef UV_WARN_2TAGS
	#define UV_WARN_2TAGS UV_WARN_2TAGS_STD
	#undef UV_DEBUG_DEV
	#define UV_DEBUG_DEV UV_DEBUG_DEV_STD
	#undef UV_WARN_DEV
	#define UV_WARN_DEV UV_WARN_DEV_STD
	#undef UV_DUMP
	#define UV_DUMP UV_DUMP_STD
	#undef UV_DUMP_DATA
	#define UV_DUMP_DATA UV_DUMP_DATA_STD
	#undef UV_ERROR
#define UV_ERROR UV_ERROR_STD
#endif

// clang-format on

#endif
