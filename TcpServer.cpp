#define UV_CLASS "TcpServer"
// #define UV_LOG_DEV_LEVEL 3

#include "TcpServer.hpp"
#include "Logger.hpp"
#include "LibUVErrors.hpp"


/* Static methods for UV callbacks. */

inline static void onConnection(uv_stream_t *handle, int status) {
	auto *server = static_cast<TcpServer*>(handle->data);

	if (server == nullptr)
		return;

	server->OnUvConnection(status);
}

inline static void onClose(uv_handle_t *handle) {
	delete handle;
}

/* Instance methods. */

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
TcpServer::TcpServer(uv_tcp_t *uvHandle, int backlog) :
		uvHandle(uvHandle) {


	int err;

	this->uvHandle->data = static_cast<void*>(this);

	err = uv_listen(reinterpret_cast<uv_stream_t*>(this->uvHandle), backlog,
			static_cast<uv_connection_cb>(onConnection));

	if (err != 0) {
		uv_close(reinterpret_cast<uv_handle_t*>(this->uvHandle),
				static_cast<uv_close_cb>(onClose));

		UV_THROW_ERROR("uv_listen() failed: %s", uv_strerror(err));
	}

	// Set local address.
	if (!SetLocalAddress()) {
		uv_close(reinterpret_cast<uv_handle_t*>(this->uvHandle),
				static_cast<uv_close_cb>(onClose));

		UV_THROW_ERROR("error setting local IP and port");
	}
}

TcpServer::~TcpServer() {


	if (!this->closed)
		Close();
}

void TcpServer::Close() {


	if (this->closed)
		return;

	this->closed = true;

	// Tell the UV handle that the TcpServer has been closed.
	this->uvHandle->data = nullptr;

	UV_DEBUG_DEV("closing %zu active connections", this->connections.size());

	for (auto *connection : this->connections) {
		delete connection;
	}

	uv_close(reinterpret_cast<uv_handle_t*>(this->uvHandle),
			static_cast<uv_close_cb>(onClose));
}

void TcpServer::Dump() const {
	UV_DUMP("<TcpServer>");
	UV_DUMP(
			"  [TCP, local:%s :%d, status:%s, connections:%zu]",
			this->localIp.c_str(),
			static_cast<uint16_t>(this->localPort),
			(!this->closed) ? "open" : "closed",
			this->connections.size());
	UV_DUMP("</TcpServer>");
}

bool TcpServer::SetLocalAddress() {


	int err;
	int len = sizeof(this->localAddr);

	err = uv_tcp_getsockname(this->uvHandle,
			reinterpret_cast<struct sockaddr*>(&this->localAddr), &len);

	if (err != 0) {
		UV_ERROR("uv_tcp_getsockname() failed: %s", uv_strerror(err));

		return false;
	}

	int family;

	GetAddressInfo(
			reinterpret_cast<const struct sockaddr*>(&this->localAddr), family,
			this->localIp, this->localPort);

	return true;
}

void TcpServer::GetAddressInfo(const struct sockaddr* addr, int& family, std::string& ip, uint16_t& port)
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

inline void TcpServer::OnUvConnection(int status) {


	if (this->closed)
		return;

	int err;

	if (status != 0) {
		UV_ERROR("error while receiving a new TCP connection: %s",
				uv_strerror(status));

		return;
	}

	// Notify the subclass so it provides an allocated derived class of TCPConnection.
	TcpConnection *connection = nullptr;
	UserOnTcpConnectionAlloc(&connection);

	UV_ASSERT(connection != nullptr,
			"TcpConnection pointer was not allocated by the user");

	try {
		connection->Setup(this, &(this->localAddr), this->localIp,
				this->localPort);
	} catch (const LibUVError &error) {
		delete connection;

		return;
	}

	// Accept the connection.
	err = uv_accept(reinterpret_cast<uv_stream_t*>(this->uvHandle),
			reinterpret_cast<uv_stream_t*>(connection->GetUvHandle()));

	if (err != 0)
		UV_ABORT("uv_accept() failed: %s", uv_strerror(err));

	// Start receiving data.
	try {
		// NOTE: This may throw.
		connection->Start();
	} catch (const LibUVError &error) {
		delete connection;

		return;
	}

	// Notify the subclass and delete the connection if not accepted by the subclass.
	if (UserOnNewTcpConnection(connection))
		this->connections.insert(connection);
	else
		delete connection;
}

inline void TcpServer::OnTcpConnectionClosed(TcpConnection *connection) {


	UV_DEBUG_DEV("TCP connection closed");

	// Remove the TcpConnection from the set.
	this->connections.erase(connection);

	// Notify the subclass.
	UserOnTcpConnectionClosed(connection);

	// Delete it.
	delete connection;
}
