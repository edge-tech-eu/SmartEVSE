#include <Arduino.h>
#include "ModBus.h"

Modbus mb = Modbus();

unsigned long next_time;

void setup() {

  Serial.begin(115200);
  delay(10000);

  Serial.printf("\r\n\r\nTesting Controlling Charger\r\nusing WaveShare 2 CH Rs495...\r\n\r\n");

  // modbus serial, mode and pins hard-coded
  mb.begin(9600);

  next_time = millis() + 1000;
}

void loop() {
  
unsigned long now = millis();

  if(now > next_time) {
    
    next_time += 5000;

    Serial.printf("Request serial and fw version\r\n");

    if (mb.readInputRegisters(1, 0, 6) == mb.ku8MBSuccess) {
      
      Serial.printf("serial: %4x%4x%4x%4x%4x, fw version: %4x\r\n",
        mb.getResponseBuffer(0),
        mb.getResponseBuffer(1),
        mb.getResponseBuffer(2),
        mb.getResponseBuffer(3),
        mb.getResponseBuffer(4),
        mb.getResponseBuffer(5));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
  }
}
