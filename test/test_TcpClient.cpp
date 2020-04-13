#include "DepLibUV.hpp"
#include "PortManager.hpp"
#include "TcpClient.hpp"
#include "TcpConnection.hpp"
#include "Timer.hpp"
#include <stdio.h>
#include <string.h>
#include <cmath>   // std::ceil()
#include <cstdio>  // sprintf()
#include <cstring> // std::memcpy(), std::memmove()

class MyTcpConnection : public TcpConnection {

public:
	MyTcpConnection(size_t size);
	virtual ~MyTcpConnection();
	void UserOnTcpConnectionRead() override;
};


MyTcpConnection::MyTcpConnection(size_t size) :
	:: TcpConnection(size){

}

MyTcpConnection::~MyTcpConnection() {

}

void MyTcpConnection::UserOnTcpConnectionRead() {
	printf("UserOnTcpConnectionRead\n");
	printf(
	  "data received [local:%s :%d, remote:%s :%d] message: %s\n",
	  GetLocalIp().c_str(),
	  GetLocalPort(),
	  GetPeerIp().c_str(),
	  GetPeerPort(), this->buffer);

}


class TcpClientTest : public TcpClient {
public:
	void UserOnTcpConnectionAlloc(TcpConnection **connection) override;
	bool UserOnNewTcpConnection(TcpConnection *connection) override;
	void UserOnTcpConnectionClosed(TcpConnection *connection) override;
};
TcpClientTest *tcpClientTest = new TcpClientTest();

void TcpClientTest::UserOnTcpConnectionAlloc(TcpConnection **connection) {
	MyTcpConnection *newTcpConnection = new MyTcpConnection((size_t)(4096));
	*connection = newTcpConnection;
	printf("UserOnTcpConnectionAlloc enter , generatet a new connection %#x\n", newTcpConnection);
}

bool TcpClientTest::UserOnNewTcpConnection(TcpConnection *connection) {
	printf("write !@!!!\n");
	char buf[1024] = {0};
	int len = 0;
	char sendBuf[1024] = {0};
	for(int i = 0; i < 100; i++) {
		len += sprintf(buf+len, "%#x", i);
	}
	printf("len = %d\n", len);
	size_t nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(len) + 1)));
	std::sprintf(sendBuf, "%zu:", len);
	std::memcpy(sendBuf + nsNumLen + 1, buf, len);
	sendBuf[nsNumLen + len + 1] = ',';
	size_t nsLen = nsNumLen + len + 2;

	connection->Write((const uint8_t *)sendBuf, nsLen, NULL);

	connection->Write((const uint8_t *)sendBuf, nsLen, NULL);

	connection->Write((const uint8_t *)sendBuf, nsLen, NULL);

	connection->Write((const uint8_t *)sendBuf, nsLen, NULL);

	return true;
}

void TcpClientTest::UserOnTcpConnectionClosed(TcpConnection *connection) {

}

class TimerTest : public Timer::Listener {
	virtual void OnTimer(Timer *timer);

};


void TimerTest::OnTimer(Timer* timer) {
	tcpClientTest->Close();
}

int main() {
	DepLibUV::ClassInit();
	std::string ip = "127.0.0.1";
	tcpClientTest->Connect(ip, 8000);
	TimerTest *timerTest = new TimerTest();
	Timer *timer = new Timer(timerTest);
	timer->Start(5000, 0);
	//close the socket after 5s
	DepLibUV::RunLoop();

	return 0;
}
