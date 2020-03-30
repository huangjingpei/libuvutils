#include <UVThread.hpp>
#include "Timer.hpp"
#include <unistd.h>

class VTThreadTest : public UVThread::Runnable ,
					public Timer::Listener
					{
public:
	virtual void run() override;
	virtual void OnTimer(Timer *timer) override;


private:

	int maxRunSeconds {5};
};

void VTThreadTest::run() {
	int count = 0;
	while (1) {
		printf("sleep %d s !\n", count);
		sleep(1);
		if (++count > maxRunSeconds) {
			break;
		}
	}
}


void VTThreadTest:: OnTimer(Timer *timer) {
	printf("timer %#x\n", timer);
}
int main() {

	auto *test = new VTThreadTest();
	auto *thread = new UVThread(test);
	thread->Join();
	printf("the thread is exited.\n");
	return 0;
}
