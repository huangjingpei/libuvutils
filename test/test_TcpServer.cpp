#include "DepLibUV.hpp"
#include "PortManager.hpp"
#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include <stdio.h>
#include <string.h>
#include <list>;
#include <algorithm>
using namespace std;
class TcpServerTest : public TcpServer {
public:
	TcpServerTest(std::string ip, uint16_t port);
	virtual ~TcpServerTest();
public:
	void UserOnTcpConnectionAlloc(TcpConnection **connection) override;
	bool UserOnNewTcpConnection(TcpConnection *connection) override;
	void UserOnTcpConnectionClosed(TcpConnection *connection) override;
};

TcpServerTest::TcpServerTest(std::string ip, uint16_t port) :
				::TcpServer(PortManager::BindTcp(ip, port), 256){

}

class MyTcpConnection : public TcpConnection {

public:
	MyTcpConnection(size_t size);
	virtual ~MyTcpConnection();
	void UserOnTcpConnectionRead() override;
};

std::list<MyTcpConnection *> myTcpConnections;

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
	this->Write((const uint8_t *)"hello ACK", strlen((const char *)"hello ACK")+1, NULL);

}

//这里不用关心释放问题
void TcpServerTest::UserOnTcpConnectionAlloc(TcpConnection **connection) {
	MyTcpConnection *newTcpConnection = new MyTcpConnection((size_t)(4096));
	*connection = newTcpConnection;
	printf("UserOnTcpConnectionAlloc enter , generatet a new connection %#x\n", newTcpConnection);
}

bool TcpServerTest::UserOnNewTcpConnection(TcpConnection *connection) {
	printf("UserOnNewTcpConnection enter, TcpConnection %#x \n", connection);
	return true;
}
void TcpServerTest::UserOnTcpConnectionClosed(TcpConnection *connection) {
	printf("UserOnTcpConnectionClosed enter, TcpConnection %#x \n", connection);



}

TcpServerTest::~TcpServerTest() {

}

int main() {
	DepLibUV::ClassInit();
	auto *tcpServerTest = new TcpServerTest("127.0.0.1", 8000);
	std::string ip {tcpServerTest->GetLocalIp() };
	uint16_t port {tcpServerTest->GetLocalPort() };

	printf("server info:%s port %d\n", ip.c_str(), port);
	DepLibUV::RunLoop();
	return 0;
}
