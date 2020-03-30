#include "UdpClient.hpp"
#include "DepLibUV.hpp"
#include "Timer.hpp"
#include <string.h>

class UdpClientTest : public UdpClient::Listener {
public:
	virtual void OnUdpSocketPacketReceived(
	  UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;
};

void UdpClientTest::OnUdpSocketPacketReceived(
		  UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) {
	printf("recv from [%s:%d] message %s\n", inet_ntoa(((struct sockaddr_in*)remoteAddr)->sin_addr),
			((struct sockaddr_in*)remoteAddr)->sin_port, data);
}

class TimerTest : public Timer::Listener {
	virtual void OnTimer(Timer *timer);

};
UdpClient *udpServer = NULL;

void TimerTest::OnTimer(Timer* timer) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("onTimer!\n");

	udpServer->Send((const uint8_t*)"hello", strlen("hello") + 1,(const struct sockaddr *)&addr, NULL);
}


int main() {
	DepLibUV::ClassInit();
	auto *udpServerTest = new UdpClientTest();
	std::string ip = "127.0.0.1";
	uint16_t port = 8001;
	udpServer = new UdpClient(udpServerTest, ip, port);
	TimerTest *timerTest = new TimerTest();
	Timer *timer = new Timer(timerTest);
	timer->Start(1000, 1000);
	DepLibUV::RunLoop();
	return 0;
}
