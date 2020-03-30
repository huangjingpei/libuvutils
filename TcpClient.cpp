#define UV_CLASS "TcpClient"
// #define UV_LOG_DEV_LEVEL 3

#include "TcpClient.hpp"
#include "Logger.hpp"
#include "LibUVErrors.hpp"
#include "DepLibUV.hpp"


/* Static methods for UV callbacks. */
inline static void onConnection(uv_connect_t *handle, int status) {
	auto *client = static_cast<TcpClient*>(handle->data);
	if (client == nullptr)
		return;

	client->OnUvConnection(status);


}

inline static void onClose(uv_handle_t *handle) {
	delete handle;
}

/* Instance methods. */

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
TcpClient::TcpClient() {

}

TcpClient::~TcpClient() {


	if (!this->closed)
		Close();
}

int TcpClient::Connect(std::string &ip, uint16_t port, int family/* = AF_INET*/) {
	int err;
	int flags { 0 };
	struct sockaddr_storage bindAddr;
	// Notify the subclass so it provides an allocated derived class of TCPConnection.
	UserOnTcpConnectionAlloc(&connection);
	UV_ASSERT(connection != nullptr,
			"TcpConnection pointer was not allocated by the user");
	try {
		connection->Setup(this, &(this->localAddr), this->localIp,
				this->localPort);
	} catch (const LibUVError &error) {
		delete connection;
		return -1;
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
	}

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


	req.data = static_cast<void*>(this);
	if (err == 0) {
		err = uv_tcp_connect(&req,
							 (uv_tcp_t*)(connection->GetUvHandle()),
							 (const struct sockaddr*) &bindAddr,
							 static_cast<uv_connect_cb>(onConnection));
	}

	// Set local address.
	if (!SetLocalAddress(&bindAddr, family)) {
		uv_close(reinterpret_cast<uv_handle_t*>(connection->GetUvHandle()),
				static_cast<uv_close_cb>(onClose));

		UV_THROW_ERROR("error setting local IP and port");
	}
	return err;
}

void TcpClient::Close() {


	if (this->closed)
		return;

	this->closed = true;

	connection->GetUvHandle()->data = nullptr;

	UV_DEBUG_DEV("closing connection");


	uv_close(reinterpret_cast<uv_handle_t*>(connection->GetUvHandle()),
			static_cast<uv_close_cb>(onClose));
}

void TcpClient::Dump() const {
	UV_DUMP("<TcpClient>");
	UV_DUMP(
			"  [TCP, local:%s :%d, status:%s]",
			this->localIp.c_str(),
			static_cast<uint16_t>(this->localPort),
			(!this->closed) ? "open" : "closed");
	UV_DUMP("</TcpClient>");
}

bool TcpClient::SetLocalAddress(struct sockaddr_storage* addr, int family/* = AF_INET*/) {

	int err;
	int len = sizeof(struct sockaddr_storage);

	err = uv_tcp_getsockname((uv_tcp_t*)connection->GetUvHandle(),
			reinterpret_cast<struct sockaddr*>(addr), &len);

	if (err != 0) {
		UV_ERROR("uv_tcp_getsockname() failed: %s", uv_strerror(err));

		return false;
	}

	GetAddressInfo(
			reinterpret_cast<const struct sockaddr*>(addr), family,
			this->localIp, this->localPort);

	return true;
}

void TcpClient::GetAddressInfo(const struct sockaddr* addr, int& family, std::string& ip, uint16_t& port)
{

	char ipBuffer[INET6_ADDRSTRLEN] = { 0 };
	int err;

	switch (addr->sa_family)
	{
		case AF_INET:
		{
			err = uv_inet_ntop(
				AF_INET, std::addressof(reinterpret_cast<const struct sockaddr_in*>(addr)->sin_addr), ipBuffer, sizeof(ipBuffer));

			if (err)
				UV_ABORT("uv_inet_ntop() failed: %s", uv_strerror(err));

			port = static_cast<uint16_t>(ntohs(reinterpret_cast<const struct sockaddr_in*>(addr)->sin_port));

			break;
		}

		case AF_INET6:
		{
			err = uv_inet_ntop(
				AF_INET6, std::addressof(reinterpret_cast<const struct sockaddr_in6*>(addr)->sin6_addr), ipBuffer, sizeof(ipBuffer));

			if (err)
				UV_ABORT("uv_inet_ntop() failed: %s", uv_strerror(err));

			port = static_cast<uint16_t>(ntohs(reinterpret_cast<const struct sockaddr_in6*>(addr)->sin6_port));

			break;
		}

		default:
		{
			UV_ABORT("unknown network family: %d", static_cast<int>(addr->sa_family));
		}
	}

	family = addr->sa_family;
	ip.assign(ipBuffer);
}

inline void TcpClient::OnUvConnection(int status) {

	if (this->closed)
		return;
	if (status != 0) {
		UV_ERROR("error while receiving a new TCP connection: %s",
				uv_strerror(status));

		return;
	}


	// Start receiving data.
	try {
		// NOTE: This may throw.
		connection->Start();
	} catch (const LibUVError &error) {
		delete connection;

		return;
	}
	// Notify the subclass only
	UserOnNewTcpConnection(connection);
}

inline void TcpClient::OnTcpConnectionClosed(TcpConnection *connection) {


	UV_DEBUG_DEV("TCP connection closed");

	// Notify the subclass.
	UserOnTcpConnectionClosed(connection);

	// Delete it.
	delete connection;
}
