#include "DepLibUV.hpp"
#include "PortManager.hpp"
#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include <stdio.h>
#include <string.h>
#include <list>;
#include <algorithm>
#include <cstdio>  // sprintf()
#include <cstring> // std::memcpy(), std::memmove()
#include "netstring.h"

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
private:
	size_t msgStart{ 0 }; // Where the latest frame starts.

};

std::list<MyTcpConnection *> myTcpConnections;

MyTcpConnection::MyTcpConnection(size_t size) :
	:: TcpConnection(size){

}

MyTcpConnection::~MyTcpConnection() {

}

void MyTcpConnection::UserOnTcpConnectionRead() {
	printf("UserOnTcpConnectionRead\n");
	// Be ready to parse more than a single message in a single TCP chunk.
			while (true)
			{
				if (IsClosed())
					return;

				size_t readLen  = this->bufferDataLen - this->msgStart;
				char* logStart = nullptr;
				size_t logLen;
				int nsRet = netstring_read(
				  reinterpret_cast<char*>(this->buffer + this->msgStart), readLen, &logStart, &logLen);
				if (nsRet != 0)
				{
					switch (nsRet)
					{
						case NETSTRING_ERROR_TOO_SHORT:
						{
							// Check if the buffer is full.
							if (this->bufferDataLen == this->bufferSize)
							{
								// First case: the incomplete message does not begin at position 0 of
								// the buffer, so move the incomplete message to the position 0.
								if (this->msgStart != 0)
								{
									std::memmove(this->buffer, this->buffer + this->msgStart, readLen);
									this->msgStart      = 0;
									this->bufferDataLen = readLen;
								}
								// Second case: the incomplete message begins at position 0 of the buffer.
								// The message is too big, so discard it.
								else
								{
									printf(
									  "no more space in the buffer for the unfinished message being parsed, "
									  "discarding it\n");

									this->msgStart      = 0;
									this->bufferDataLen = 0;
								}
							}

							// Otherwise the buffer is not full, just wait.
							return;
						}

						case NETSTRING_ERROR_TOO_LONG:
						{
							printf("NETSTRING_ERROR_TOO_LONG\n");

							break;
						}

						case NETSTRING_ERROR_NO_COLON:
						{
							printf("NETSTRING_ERROR_NO_COLON\n");

							break;
						}

						case NETSTRING_ERROR_NO_COMMA:
						{
							printf("NETSTRING_ERROR_NO_COMMA\n");

							break;
						}

						case NETSTRING_ERROR_LEADING_ZERO:
						{
							printf("NETSTRING_ERROR_LEADING_ZERO\n");

							break;
						}

						case NETSTRING_ERROR_NO_LENGTH:
						{
							printf("NETSTRING_ERROR_NO_LENGTH\n");

							break;
						}
					}

					// Error, so reset and exit the parsing loop.
					this->msgStart      = 0;
					this->bufferDataLen = 0;

					return;
				}
				//printf("AAAAAAAA! logLen %d logStart %p\n", logLen, logStart);
				// If here it means that jsonStart points to the beginning of a JSON string
				// with logLen bytes length, so recalculate readLen.
				readLen =
				  reinterpret_cast<const uint8_t*>(logStart) - (this->buffer + this->msgStart) + logLen + 1;

				{
					if (logStart[logLen] == ',') {
						printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
						logStart[logLen] = '\0';
					}
					printf("recv : %s\n", logStart);
					printf("-----------------------------------------------------------\n");
				}


				// If there is no more space available in the buffer and that is because
				// the latest parsed message filled it, then empty the full buffer.
				if ((this->msgStart + readLen) == this->bufferSize)
				{
					this->msgStart      = 0;
					this->bufferDataLen = 0;
				}
				// If there is still space in the buffer, set the beginning of the next
				// parsing to the next position after the parsed message.
				else
				{
					this->msgStart += readLen;
				}

				// If there is more data in the buffer after the parsed message
				// then parse again. Otherwise break here and wait for more data.
				if (this->bufferDataLen > this->msgStart)
				{
					continue;
				}

				break;
			}

}

//这里不用关心释放问题
void TcpServerTest::UserOnTcpConnectionAlloc(TcpConnection **connection) {
	MyTcpConnection *newTcpConnection = new MyTcpConnection((size_t)(4096));
	*connection = newTcpConnection;
	printf("UserOnTcpConnectionAlloc enter , generatet a new connection %#x\n", newTcpConnection);
}

bool TcpServerTest::UserOnNewTcpConnection(TcpConnection *connection) {
	//这里如果想拒绝对方链接的，直接返回false
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
