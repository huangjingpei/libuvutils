#ifndef MS_TCP_CLIENT_HPP
#define MS_TCP_CLIENT_HPP


#include <uv.h>
#include <string>
#include <unordered_set>

#include "TcpConnection.hpp"

class TcpClient: public TcpConnection::Listener {
public:
	/**
	 * uvHandle must be an already initialized and binded uv_tcp_t pointer.
	 */
	TcpClient();
	virtual ~TcpClient() override;

public:
	int Connect(std::string &ip, uint16_t port, int family = AF_INET);
	void Close();
	virtual void Dump() const;
	const struct sockaddr* GetLocalAddress() const;
	int GetLocalFamily() const;
	const std::string& GetLocalIp() const;
	uint16_t GetLocalPort() const;
	TcpConnection* GetConnection() const;

protected:
	void AcceptTcpConnection(TcpConnection* connection);

private:
	bool SetLocalAddress(struct sockaddr_storage* addr, int family = AF_INET);
	void GetAddressInfo(const struct sockaddr* addr, int& family, std::string& ip, uint16_t& port);

	/* Pure virtual methods that must be implemented by the subclass. */
protected:
	virtual void UserOnTcpConnectionAlloc(TcpConnection **connection) = 0;
	virtual bool UserOnNewTcpConnection(TcpConnection *connection) = 0;
	virtual void UserOnTcpConnectionClosed(TcpConnection *connection) = 0;

	/* Callbacks fired by UV events. */
public:
	void OnUvConnection(int status);

	/* Methods inherited from TcpConnection::Listener. */
public:
	void OnTcpConnectionClosed(TcpConnection *connection) override;

protected:
	struct sockaddr_storage localAddr;
	std::string localIp;
	uint16_t localPort { 0 };

private:

	// Others.
	TcpConnection *connection { nullptr };
	uv_connect_t req;
	bool closed { false };
};

/* Inline methods. */

inline TcpConnection* TcpClient::GetConnection() const {
	return connection;
}

inline const struct sockaddr* TcpClient::GetLocalAddress() const {
	return reinterpret_cast<const struct sockaddr*>(&this->localAddr);
}

inline int TcpClient::GetLocalFamily() const {
	return reinterpret_cast<const struct sockaddr*>(&this->localAddr)->sa_family;
}

inline const std::string& TcpClient::GetLocalIp() const {
	return this->localIp;
}

inline uint16_t TcpClient::GetLocalPort() const {
	return this->localPort;
}

#endif //MS_TCP_CLIENT_HPP
