#define UV_CLASS "UdpServer"

#include "UdpServer.hpp"
#include "UdpSocket.hpp"
#include "LibUVErrors.hpp"
#include "PortManager.hpp"

UdpServer::UdpServer(Listener* listener, std::string &ip, uint16_t port) :
 	 :: UdpSocket(PortManager::BindUdp(ip, port))// This may throw.
	, listener(listener){

}

UdpServer::~UdpServer() {
	PortManager::UnbindUdp(this->localIp, this->localPort);
}

void UdpServer::UserOnUdpDatagramReceived(const uint8_t *data, size_t len,
				const struct sockaddr *addr) {
	if (this->listener == nullptr)
	{
		UV_ERROR("no listener set");
		return;
	}

	this->listener->OnUdpSocketPacketReceived(this, data, len, addr);

}
