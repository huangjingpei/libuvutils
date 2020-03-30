#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP
#include "UdpSocket.hpp"
class UdpClient : public UdpSocket {
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
	UdpClient(Listener *listener, std::string &ip, uint16_t port);
	virtual ~UdpClient();
public:
	virtual void UserOnUdpDatagramReceived(const uint8_t *data, size_t len,
				const struct sockaddr *addr) override;
private:
	Listener* listener{ nullptr };


};
#endif//UDP_CLIENT_HPP
