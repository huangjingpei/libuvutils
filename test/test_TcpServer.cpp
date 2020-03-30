#include "DepLibUV.hpp"
#include "PortManager.hpp"
#include "TcpServer.hpp"
#include "TcpConnection.hpp"

class TcpServerTest : TcpServer {
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


class MyTcpConnection : TcpConnection {

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
}

void TcpServerTest::UserOnTcpConnectionAlloc(TcpConnection **connection) {
	MyTcpConnection *newTcpConnection = new MyTcpConnection((size_t)(4096));
	*connection = (TcpConnection*)newTcpConnection;
	printf("UserOnNewTcpConnection enter\n");

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
	DepLibUV::RunLoop();
	return 0;
}
