#define UV_CLASS "RTC::PortManager"
// #define UV_LOG_DEV_LEVEL 3

#include "PortManager.hpp"
#include "DepLibUV.hpp"
#include "Logger.hpp"
#include "LibUVErrors.hpp"

#include <utility> // std::piecewise_construct

/* Static methods for UV callbacks. */
#define PORT_RANGE_START 52000
#define PORT_RANGE_END   59999
#define BIND_ADDTEMPTS  100

static inline void onClose(uv_handle_t *handle) {
	delete handle;
}

inline static void onFakeConnection(uv_stream_t* /*handle*/, int /*status*/) {
	// Do nothing.
}

/* Class variables. */

std::unordered_map<std::string, std::vector<bool>> PortManager::mapUdpIpPorts;
std::unordered_map<std::string, std::vector<bool>> PortManager::mapTcpIpPorts;

/* Class methods. */

uv_handle_t* PortManager::Bind(Transport transport, std::string &ip) {

	// First normalize the IP. This may throw if invalid IP.
	NormalizeIp(ip);

	int err;
	int family = GetFamily(ip);
	struct sockaddr_storage bindAddr; // NOLINT(cppcoreguidelines-pro-type-member-init)
	size_t portIdx;
	int flags { 0 };
	size_t attempt { 0u };
	size_t numAttempts { BIND_ADDTEMPTS };
	uv_handle_t *uvHandle { nullptr };
	uint16_t port;
	std::string transportStr;

	switch (transport) {
	case Transport::UDP:
		transportStr.assign("udp");
		break;

	case Transport::TCP:
		transportStr.assign("tcp");
		break;
	}

	switch (family) {
	case AF_INET: {
		err = uv_ip4_addr(ip.c_str(), 0,
				reinterpret_cast<struct sockaddr_in*>(&bindAddr));

		if (err != 0)
			UV_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

		break;
	}

	case AF_INET6: {
		err = uv_ip6_addr(ip.c_str(), 0,
				reinterpret_cast<struct sockaddr_in6*>(&bindAddr));

		if (err != 0)
			UV_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

		// Don't also bind into IPv4 when listening in IPv6.
		flags |= UV_UDP_IPV6ONLY;

		break;
	}

		// This cannot happen.
	default: {
		UV_ABORT("unknown IP family");
	}
	}

	// Choose a random port index to start from.
	portIdx = rand() % 8000 + PORT_RANGE_START;

	// Iterate all ports until getting one available. Fail if none found and also
	// if bind() fails N times in theorically available ports.
	while (true) {
		// Increase attempt number.
		++attempt;

		// If we have tried all the ports in the range throw.
		if (attempt > numAttempts) {
			UV_THROW_ERROR(
					"no more available ports [transport:%s, ip:%s, numAttempt:%zu]",
					transportStr.c_str(), ip.c_str(), numAttempts);
		}

		// Increase current port index.
		portIdx = (portIdx + 1) % 60000;
		if (portIdx < PORT_RANGE_START) {
			portIdx += PORT_RANGE_START;
		}

		// So the corresponding port is the vector position plus the RTC minimum port.
		port = static_cast<uint16_t>(portIdx);

		UV_DEBUG_DEV(
				"testing port [transport:%s, ip:%s, port:%" PRIu16 ", attempt:%zu/%zu]",
				transportStr.c_str(),
				ip.c_str(),
				port,
				attempt,
				numAttempts);

		// Check whether this port is not available.
		if (false) {	//TODO jphuang need do something
			UV_DEBUG_DEV(
					"port in use, trying again [transport:%s, ip:%s, port:%" PRIu16 ", attempt:%zu/%zu]",
					transportStr.c_str(),
					ip.c_str(),
					port,
					attempt,
					numAttempts);

			continue;
		}

		// Here we already have a theorically available port. Now let's check
		// whether no other process is binding into it.

		// Set the chosen port into the sockaddr struct.
		switch (family) {
		case AF_INET:
			(reinterpret_cast<struct sockaddr_in*>(&bindAddr))->sin_port =
					htons(port);
			break;

		case AF_INET6:
			(reinterpret_cast<struct sockaddr_in6*>(&bindAddr))->sin6_port =
					htons(port);
			break;
		}

		// Try to bind on it.
		switch (transport) {
		case Transport::UDP:
			uvHandle = reinterpret_cast<uv_handle_t*>(new uv_udp_t());
			err = uv_udp_init(DepLibUV::GetLoop(),
					reinterpret_cast<uv_udp_t*>(uvHandle));
			break;

		case Transport::TCP:
			uvHandle = reinterpret_cast<uv_handle_t*>(new uv_tcp_t());
			err = uv_tcp_init(DepLibUV::GetLoop(),
					reinterpret_cast<uv_tcp_t*>(uvHandle));
			break;
		}

		if (err != 0) {
			delete uvHandle;

			switch (transport) {
			case Transport::UDP:
				UV_THROW_ERROR("uv_udp_init() failed: %s", uv_strerror(err));
				break;

			case Transport::TCP:
				UV_THROW_ERROR("uv_tcp_init() failed: %s", uv_strerror(err));
				break;
			}
		}

		switch (transport) {
		case Transport::UDP: {
			err = uv_udp_bind(reinterpret_cast<uv_udp_t*>(uvHandle),
					reinterpret_cast<const struct sockaddr*>(&bindAddr), flags);

			UV_WARN_DEV(
					"uv_udp_bind() failed [transport:%s, ip:%s, port:%" PRIu16 ", attempt:%zu/%zu]: %s",
					transportStr.c_str(),
					ip.c_str(),
					port,
					attempt,
					numAttempts,
					uv_strerror(err));

			break;
		}

		case Transport::TCP: {
			err = uv_tcp_bind(reinterpret_cast<uv_tcp_t*>(uvHandle),
					reinterpret_cast<const struct sockaddr*>(&bindAddr), flags);

			if (err) {
				UV_WARN_DEV(
						"uv_tcp_bind() failed [transport:%s, ip:%s, port:%" PRIu16 ", attempt:%zu/%zu]: %s",
						transportStr.c_str(),
						ip.c_str(),
						port,
						attempt,
						numAttempts,
						uv_strerror(err));
			}

			// uv_tcp_bind() may succeed even if later uv_listen() fails, so
			// double check it.
			if (err == 0) {
				err = uv_listen(reinterpret_cast<uv_stream_t*>(uvHandle), 256,
						static_cast<uv_connection_cb>(onFakeConnection));

				UV_WARN_DEV(
						"uv_listen() failed [transport:%s, ip:%s, port:%d, attempt:%zu/%zu]: %s",
						transportStr.c_str(),
						ip.c_str(),
						port,
						attempt,
						numAttempts,
						uv_strerror(err));
			}

			break;
		}
		}

		// If it succeeded, exit the loop here.
		if (err == 0)
			break;

		// If it failed, close the handle and check the reason.
		uv_close(reinterpret_cast<uv_handle_t*>(uvHandle),
				static_cast<uv_close_cb>(onClose));

		switch (err) {
		// If bind() fails due to "too many open files" just throw.
		case UV_EMFILE: {
			UV_THROW_ERROR(
					"port bind failed due to too many open files [transport:%s, ip:%s, port:%d , attempt:%zu/%zu]",
					transportStr.c_str(), ip.c_str(), port, attempt,
					numAttempts);

			break;
		}

			// If cannot bind in the given IP, throw.
		case UV_EADDRNOTAVAIL: {
			UV_THROW_ERROR(
					"port bind failed due to address not available [transport:%s, ip:%s, port:%d , attempt:%zu/%zu]",
					transportStr.c_str(), ip.c_str(), port, attempt,
					numAttempts);

			break;
		}

		default: {
			// Otherwise continue in the loop to try again with next port.
		}
		}
	}

// If here, we got an available port. Mark it as unavailable.

	UV_DEBUG_DEV(
			"bind succeeded [transport:%s, ip:%s, port:%" PRIu16 ", attempt:%zu/%zu]",
			transportStr.c_str(),
			ip.c_str(),
			port,
			attempt,
			numAttempts);

	return static_cast<uv_handle_t*>(uvHandle);
}

void PortManager::Unbind(Transport transport, std::string &ip, uint16_t port) {

	if ((static_cast<size_t>(port) < PORT_RANGE_START)
			|| (static_cast<size_t>(port) > PORT_RANGE_END)) {
		UV_ERROR("given port %d  is out of range", port)
		;

		return;
	}
}

void PortManager::NormalizeIp(std::string &ip) {

	static sockaddr_storage addrStorage;
	char ipBuffer[INET6_ADDRSTRLEN] = { 0 };
	int err;

	switch (GetFamily(ip)) {
	case AF_INET: {
		err = uv_ip4_addr(ip.c_str(), 0,
				reinterpret_cast<struct sockaddr_in*>(&addrStorage));

		if (err != 0)
			UV_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

		err = uv_ip4_name(
				reinterpret_cast<const struct sockaddr_in*>(std::addressof(
						addrStorage)), ipBuffer, sizeof(ipBuffer));

		if (err != 0)
			UV_ABORT("uv_ipv4_name() failed: %s", uv_strerror(err));

		ip.assign(ipBuffer);

		break;
	}

	case AF_INET6: {
		err = uv_ip6_addr(ip.c_str(), 0,
				reinterpret_cast<struct sockaddr_in6*>(&addrStorage));

		if (err != 0)
			UV_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

		err = uv_ip6_name(
				reinterpret_cast<const struct sockaddr_in6*>(std::addressof(
						addrStorage)), ipBuffer, sizeof(ipBuffer));

		if (err != 0)
			UV_ABORT("uv_ip6_name() failed: %s", uv_strerror(err));

		ip.assign(ipBuffer);

		break;
	}

	default: {
		UV_THROW_TYPE_ERROR("invalid ip '%s'", ip.c_str());
	}
	}
}

uv_handle_t* PortManager::Bind(Transport transport, std::string &ip,
		uint16_t port) {

	// First normalize the IP. This may throw if invalid IP.
	NormalizeIp(ip);

	int err;
	int family = GetFamily(ip);
	struct sockaddr_storage bindAddr; // NOLINT(cppcoreguidelines-pro-type-member-init)
	int flags { 0 };
	uv_handle_t *uvHandle { nullptr };
	std::string transportStr;

	switch (transport) {
	case Transport::UDP:
		transportStr.assign("udp");
		break;

	case Transport::TCP:
		transportStr.assign("tcp");
		break;
	}

	switch (family) {
	case AF_INET: {
		err = uv_ip4_addr(ip.c_str(), port,
				reinterpret_cast<struct sockaddr_in*>(&bindAddr));

		if (err != 0)
			UV_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

		break;
	}

	case AF_INET6: {
		err = uv_ip6_addr(ip.c_str(), port,
				reinterpret_cast<struct sockaddr_in6*>(&bindAddr));

		if (err != 0)
			UV_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

		// Don't also bind into IPv4 when listening in IPv6.
		flags |= UV_UDP_IPV6ONLY;

		break;
	}

		// This cannot happen.
	default: {
		UV_ABORT("unknown IP family");
	}
	}

	// Iterate all ports until getting one available. Fail if none found and also
	// if bind() fails N times in theorically available ports.
	do {
		// Try to bind on it.
		switch (transport) {
		case Transport::UDP:
			uvHandle = reinterpret_cast<uv_handle_t*>(new uv_udp_t());
			err = uv_udp_init(DepLibUV::GetLoop(),
					reinterpret_cast<uv_udp_t*>(uvHandle));
			break;

		case Transport::TCP:
			uvHandle = reinterpret_cast<uv_handle_t*>(new uv_tcp_t());
			err = uv_tcp_init(DepLibUV::GetLoop(),
					reinterpret_cast<uv_tcp_t*>(uvHandle));
			break;
		}

		if (err != 0) {
			delete uvHandle;

			switch (transport) {
			case Transport::UDP:
				UV_THROW_ERROR("uv_udp_init() failed: %s", uv_strerror(err));
				break;

			case Transport::TCP:
				UV_THROW_ERROR("uv_tcp_init() failed: %s", uv_strerror(err));
				break;
			}
		}

		switch (transport) {
		case Transport::UDP: {
			err = uv_udp_bind(reinterpret_cast<uv_udp_t*>(uvHandle),
					reinterpret_cast<const struct sockaddr*>(&bindAddr), flags);

			UV_WARN_DEV(
					"uv_udp_bind() failed [transport:%s, ip:%s, port:%d]: %s",
					transportStr.c_str(),
					ip.c_str(),
					port,
					uv_strerror(err));

			break;
		}

		case Transport::TCP: {
			err = uv_tcp_bind(reinterpret_cast<uv_tcp_t*>(uvHandle),
					reinterpret_cast<const struct sockaddr*>(&bindAddr), flags);

			if (err) {
				UV_WARN_DEV(
						"uv_tcp_bind() failed [transport:%s, ip:%s, port:%d]: %s",
						transportStr.c_str(),
						ip.c_str(),
						port,
						uv_strerror(err));
			}

			// uv_tcp_bind() may succeed even if later uv_listen() fails, so
			// double check it.
			if (err == 0) {
				err = uv_listen(reinterpret_cast<uv_stream_t*>(uvHandle), 256,
						static_cast<uv_connection_cb>(onFakeConnection));

				UV_WARN_DEV(
						"uv_listen() failed [transport:%s, ip:%s, port:%d]: %s",
						transportStr.c_str(),
						ip.c_str(),
						port,
						uv_strerror(err));
			}

			break;
		}
		}

		// If it succeeded, exit the loop here.
		if (err == 0)
			break;

		// If it failed, close the handle and check the reason.
		uv_close(reinterpret_cast<uv_handle_t*>(uvHandle),
				static_cast<uv_close_cb>(onClose));

		switch (err) {
		// If bind() fails due to "too many open files" just throw.
		case UV_EMFILE: {
			UV_THROW_ERROR(
					"port bind failed due to too many open files [transport:%s, ip:%s, port:%d]",
					transportStr.c_str(), ip.c_str(), port);

			break;
		}

			// If cannot bind in the given IP, throw.
		case UV_EADDRNOTAVAIL: {
			UV_THROW_ERROR(
					"port bind failed due to address not available [transport:%s, ip:%s, port:%d]",
					transportStr.c_str(), ip.c_str(), port);

			break;
		}

		default: {
			// Otherwise continue in the loop to try again with next port.
		}
		}
	} while (0);

// If here, we got an available port. Mark it as unavailable.

	UV_DEBUG_DEV(
			"bind succeeded [transport:%s, ip:%s, port:%d]",
			transportStr.c_str(),
			ip.c_str(),
			port);

	return static_cast<uv_handle_t*>(uvHandle);
}

/**

 1.判断字符串是否形如“192.168.1.1”

 2.字符串两端含有空格视为合法ip，形如“    192.168.1.1    ”

 3.字符串中间含有空格视为非法ip，形如“192.168. 1.2”

 4.字符串0开头视为不合法ip，形如192.168.01.1

 5.字符串0.0.0.0视为合法ip
 */

int PortManager::GetFamily(std::string &ipstring) {
	const char *ip = ipstring.c_str();
	if (NULL == ip) {
		return false;
	}
	const char *q = ip;     //字串指针
	unsigned short int s = 0, count = 0, digitNumber = 0; //s是字串转化为的整型，count是 . 的个数, digitNumber 是 . 之间的数量
	bool hasZero = false;
// 开头有空格
	while (' ' == *q) {
		q++;
	}

	while ('\0' != *q) {
		if ('.' == *q) {
			// . 前面没有任何值，则非法
			if (digitNumber == 0) {
				return false;
			}
			s = 0;
			digitNumber = 0;
			count++;

			hasZero = false;

			q++;

			continue;
		}

		// 值非法
		if (*q < '0' || *q > '9') {
			// 结尾空格
			if (' ' == *q && 3 == count) {
				const char *qq = q;
				while (' ' == *qq) {
					qq++;
				}
				return '\0' == *qq;
			} else {
				return false;
			}
		}

		int x = *q - '0';
		s = s * 10 + x;

		// 0.0.0.0 合法， 00.0.0.0 不合法
		if (0 == s) {
			if (hasZero) {
				return false;
			} else {
				hasZero = true;
			}
		}

		if (s > 255) {
			return false;
		}
		digitNumber++;
		q++;
	}

	if (3 == count) {
		return AF_INET;
	} else {
		return AF_INET6;
	}
}

/*
 int main(void){
 {
 const int count = 10;
 char *ip[count] = {"0.0.0.0", "255.255.255.255", "0.10.0.0", " 1.1.1.1", "1.1.1.1 ", " 1.1.1.1 "};
 for(int i = 0; i < count; i++) {
 if(checkIpv4(ip[i]))
 printf("该地址是IPv4地址\n");
 else
 printf("该地址不是IPv4地址\n");
 }
 }
 printf("\n\n");
 {
 const int count = 10;
 char *ip[count] = {"1.1.1. 1", "1..2.3", "00.1.1.1", "a.1.1.1", };
 for(int i = 0; i < count; i++) {
 if(checkIpv4(ip[i]))
 printf("该地址是IPv4地址\n");
 else
 printf("该地址不是IPv4地址\n");
 }
 }
 return 0;
 }
 */

