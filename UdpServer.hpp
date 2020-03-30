#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP
#include "UdpSocket.hpp"
class UdpServer : public UdpSocket {
public:
	class Listener
	{
	public:
		Listener() {};
		virtual ~Listener() {};
	public:
		virtual void OnUdpSocketPacketReceived(
		  UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) = 0;
	};

public:
	UdpServer(Listener* listener, std::string &ip, uint16_t port);
	virtual ~UdpServer();
public:
	virtual void UserOnUdpDatagramReceived(const uint8_t *data, size_t len,
				const struct sockaddr *addr) override;

private:
	// Passed by argument.
	Listener* listener{ nullptr };

};
#endif//UDP_SERVER_HPP
