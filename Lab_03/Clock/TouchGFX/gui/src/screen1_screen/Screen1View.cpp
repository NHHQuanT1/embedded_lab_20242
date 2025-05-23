#include <gui/screen1_screen/Screen1View.hpp>
#include <cmsis_os.h>
#include <main.h>
#include <math.h>
#include <stm32f429xx.h>

Screen1View::Screen1View() {

}

void Screen1View::setupScreen() {
	Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen() {
	Screen1ViewBase::tearDownScreen();
}

extern osMessageQueueId_t Queue1Handle;
void Screen1View::handleTickEvent() {
	static uint32_t prevTick = 0;
//	static uint32_t prevSystemTick = 0;
	Screen1ViewBase::handleTickEvent();
	tickCount++;
	float rad = 0;

	uint8_t res = 0;
	uint32_t count = osMessageQueueGetCount(Queue1Handle);
//	uint32_t currentTick = osKernelGetTickCount();

	if (count > 0) {
		osMessageQueueGet(Queue1Handle, &res, NULL, osWaitForever);
		if (res == 'P') {
//			uint32_t elapsed = currentTick - prevSystemTick;
//			float seconds = elapsed / 1000.0f;
//			rad = fmodf(seconds * (3.14f / 30.0f), 2.0f * 3.14f);
			rad = ((tickCount - prevTick) % 360) * 3.14f / 180;
			txtrHand.updateZAngle(rad);
//			prevSystemTick = currentTick;
//			prevTick = tickCount;
		}
	}
	else {
		prevTick = tickCount;
//		prevSystemTick = currentTick;
	}
//
//		Screen1ViewBase::handleTickEvent();
//		tickCount++;
//		float rad = (tickCount % 360)*3.14f/180;
//
//		txtrHand.updateZAngle(rad);

}
