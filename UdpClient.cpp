#define UV_CLASS "UdpClient"

#include "UdpClient.hpp"
#include "UdpSocket.hpp"
#include "LibUVErrors.hpp"
#include "PortManager.hpp"

UdpClient::UdpClient(Listener* listener, std::string &ip, uint16_t port) :
 	 :: UdpSocket(PortManager::BindUdp(ip, port))// This may throw.
	, listener(listener){

}

UdpClient::~UdpClient() {
	PortManager::UnbindUdp(this->localIp, this->localPort);
}

void UdpClient::UserOnUdpDatagramReceived(const uint8_t *data, size_t len,
				const struct sockaddr *addr) {
	if (this->listener == nullptr)
	{
		UV_ERROR("no listener set");
		return;
	}

	this->listener->OnUdpSocketPacketReceived(this, data, len, addr);

}
