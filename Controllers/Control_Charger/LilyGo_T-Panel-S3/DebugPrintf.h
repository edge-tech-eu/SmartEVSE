#ifdef ARDUINO_ARCH_RP2040

	#define DEBUG_PRINTF	Serial.printf

#elif ARDUINO_ESP32S3_DEV

	#define DEBUG_PRINTF	Serial.printf

#else

	#error ModBus.cpp needs to be adapted to architecture and rs485 interface, please check/add your combination

#endif