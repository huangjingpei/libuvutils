#include "DepLibUV.hpp"
#include "PortManager.hpp"
#include "TcpClient.hpp"
#include "TcpConnection.hpp"
#include "Timer.hpp"
#include <stdio.h>
#include <string.h>


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
	connection->Write((const uint8_t *)"hello", strlen((const char *)"hello")+1, NULL);
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
