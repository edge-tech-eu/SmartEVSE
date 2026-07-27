#ifndef BOARD_h
#define BOARD_h

// this is file is included in all general files
// but is specific for:
// 
// M5Stack M5 Core SE + Commu + LAN Module13.2 + DIN Base

#define DEBUG_PRINTF	Serial.printf

void board_init(void);
void board_system_check(void);

#endif