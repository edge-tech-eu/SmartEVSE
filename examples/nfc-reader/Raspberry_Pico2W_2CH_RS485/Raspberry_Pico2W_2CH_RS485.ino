/*

  NFC card reader EVSE-X-NFC: print the ID of every card that is held against the reader

  Raspberry Pi Pico 2W + WaveShare 2-Channel RS485 Module for Pico
  Board: Raspberry Pi Pico/RP2040/RP2350: Raspberry Pi Pico 2W

    Connections:
    NFC reader:           WaveShare:
    A                     Channel 1: A
    B                     Channel 1: B
    - (GND)               Channel 1: G
    + and -               12 V DC supply (or 5 V DC on the 5V header pin)

  The reader polls nothing by itself: the master (this sketch, or an EdgeTech site controller)
  asks it about once a second whether a new card was read.

  Documentation:
  - https://github.com/edge-tech-eu/SmartEVSE/blob/main/docs/en/nfc-reader.md
  - https://www.waveshare.com/pico-2ch-rs485.htm

    your friends @ EdgeTech

*/
#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"

// Modbus address of the reader. EdgeTech readers are set to 50; change it with the maker's
// configuration program over USB (docs/en/nfc-reader.md).
#define READER_ADDRESS 50

// Baud rate: the same as the reader is set to (and as the rest of the bus).
#define BAUD 9600

// Register and coil addresses as sent on the wire (0-based). The maker's manual numbers them
// from 1: holding register 1000 in the manual is address 999 here, coil 1016 is 1015.
#define REG_IS_NEW_TAG   999   // 1 = a new card was read
#define REG_UID_LENGTH  1000   // UID length in bytes: 4, 7 or 10
#define REG_UID         1001   // UID, one byte per register (up to 10 registers)
#define REG_MODEL       1016   // model ID, then software version, then hardware version
#define COIL_IS_NEW_TAG 1015   // write 0 to clear the flag; the reader then reads the next card

Modbus mb;

unsigned long next_time;


bool reader_detect() {

  if (mb.readHoldingRegisters(READER_ADDRESS, REG_MODEL, 3) != mb.ku8MBSuccess) {
    return false;
  }
  Serial.printf("reader found at address %d: model %x, software %x, hardware %x\r\n",
                READER_ADDRESS, mb.getResponseBuffer(0), mb.getResponseBuffer(1), mb.getResponseBuffer(2));
  return true;
}


void reader_clear() {

  mb.writeSingleCoil(READER_ADDRESS, COIL_IS_NEW_TAG, 0);
}


// Returns true and fills uid (hex, 0-terminated) when a new card was read.
bool reader_new_card(char *uid, size_t size) {

  // flag, length and up to 10 UID bytes in one request: 12 registers
  if (mb.readHoldingRegisters(READER_ADDRESS, REG_IS_NEW_TAG, 12) != mb.ku8MBSuccess) {
    return false;
  }
  if (mb.getResponseBuffer(0) != 1) {
    return false;
  }
  int len = mb.getResponseBuffer(1);
  if (len < 0) len = 0;
  if (len > 10) len = 10;
  if ((size_t)(len * 2 + 1) > size) len = (size - 1) / 2;
  for (int i = 0; i < len; i++) {
    snprintf(uid + i * 2, 3, "%02X", mb.getResponseBuffer(2 + i) & 0xFF);
  }
  uid[len * 2] = 0;
  return true;
}


void setup() {

  Serial.begin(115200);

  delay(10000);

  Serial.printf("\r\n\r\nNFC card reader EVSE-X-NFC\r\nusing WaveShare 2 CH RS485...\r\n\r\n");

  mb.begin(BAUD);

  if (!reader_detect()) {
    Serial.printf("no reader at address %d: check power, A/B, GND, address and baud rate\r\n", READER_ADDRESS);
  }

  delay(100);
  reader_clear();

  next_time = millis() + 1000;
}


void loop() {

  unsigned long now = millis();

  if (now > next_time) {

    next_time += 1000;

    char uid[24];

    if (reader_new_card(uid, sizeof(uid))) {

      Serial.printf("card: %s\r\n", uid);

      // Your own logic goes here, for example: compare uid with a whitelist and allow the
      // charger to start. Then clear the flag so the reader reads the next card.
      delay(100);
      reader_clear();
    }
  }
}
