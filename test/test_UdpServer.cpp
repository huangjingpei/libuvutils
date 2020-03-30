#include "DepLibUV.hpp"
#include "UdpServer.hpp"
#include "UdpSocket.hpp"

class UdpServerTest : public UdpServer::Listener {
public:
	virtual void OnUdpSocketPacketReceived(
	  UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;
};

void UdpServerTest::OnUdpSocketPacketReceived(UdpSocket* socket,
		const uint8_t* data,
		size_t len,
		const struct sockaddr* remoteAddr) {
	printf("recv from [%s:%d] message %s\n", inet_ntoa(((struct sockaddr_in*)remoteAddr)->sin_addr),
			((struct sockaddr_in*)remoteAddr)->sin_port, data);


}

int main() {
	DepLibUV::ClassInit();
	auto *udpServerTest = new UdpServerTest();
	std::string ip = "127.0.0.1";
	auto *udpServer = new UdpServer(udpServerTest, ip, 8000);
	DepLibUV::RunLoop();
	return 0;
}


