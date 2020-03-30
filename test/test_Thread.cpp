#include <UVThread.hpp>
#include "Timer.hpp"
#include <unistd.h>

class UVThreadTest : public UVThread::Runnable
					{
public:
	virtual void run() override;
private:

	int maxRunSeconds {5};
};

void UVThreadTest::run() {
	int count = 0;
	while (1) {
		printf("sleep %d s !\n", count);
		sleep(1);
		if (++count > maxRunSeconds) {
			break;
		}
	}
}

int main() {

	auto *test = new UVThreadTest();
	auto *thread = new UVThread(test);
	thread->Join();
	printf("the thread is exited.\n");
	return 0;
}
