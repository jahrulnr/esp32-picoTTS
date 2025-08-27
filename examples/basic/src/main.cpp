#include "app.h"

void setup(){
	setupSpeakers();
	setupPicoTTS();
}

static const char* msgs[] = {
	"Hello, world!",
	"ESP32 is running smoothly",
	"Have a great day!",
	"Text to speech is working perfectly",
	"Ready for voice synthesis",
	"Microcontroller is online",
	"Audio output functioning normally",
	"Thank you for using this device"
};

void loop(){
	static int index = 0;
	sayText(msgs[index]);
	index = (index + 1) % (sizeof(msgs) / sizeof(msgs[0]));

	delay(10000);
}