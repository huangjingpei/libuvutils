#include <unistd.h>

#include "../Thread.hpp"
#include "Timer.hpp"
#include "DepLibUV.hpp"

class TimerTest : public Timer::Listener {
	virtual void OnTimer(Timer *timer);

};

int count = 0;
void TimerTest::OnTimer(Timer* timer) {
	printf("count = %d peroid %zu timeout %zu\n",++count, timer->GetRepeat(), timer->GetTimeout());
	if (count == 5) {
		timer->Close();
	}
}


#define TIMER_FIRST_RUN 1001 //ms
#define TIMER_PEROID 1000 //ms

int main() {
	DepLibUV::ClassInit();
	TimerTest *timerTest = new TimerTest();
	Timer *timer = new Timer(timerTest);
	timer->Start(TIMER_FIRST_RUN, TIMER_PEROID);
	DepLibUV::RunLoop();
	return 0;
}
