#include <M5Unified.h>
#include <M5GFX.h>
#include "Board.h"
#include "DSMR5Reader.h"
#include "UI.h"

void board_init() {

  auto cfg = M5.config();
  cfg.internal_imu = false;
  cfg.internal_mic = false;
  cfg.clear_display = true;
  M5.begin(cfg);

  m5::board_t board = M5.getBoard();
  switch (board) {
    case m5::board_t::board_M5StackCoreS3SE:
      {
        Serial.printf("Found M5StackCoreS3SE\r\n\r\n");
      }
      break;
    default:
      {
        Serial.printf("Firmware is for M5StackCoreS3SE!!! (this is a %d)\r\n\r\n", board);
      }
      break;
  }
}

void board_system_check(void) {

  bool error;
  
  char line[80];

  do {

    error = false;

    ui_clear();

    ui_start_up();

    ui_start_up_add_line("\r\n- Connecting to charger: ");

    dsmr5reader_init();

    int meter_status = dsmr5reader_check();
    if (meter_status != DSMR_OK) {

      sprintf(line, "- No compatible meter found! (code: %d)\r\n", meter_status);
      ui_start_up_add_line_error(line);
      error = true;
      
    } else {

      sprintf(line, "- DSMR: %s\r\n", dsmr_id);
      ui_start_up_add_line(line);
      if (dsmr_single_phase) {
        sprintf(line, "- Singel phase system\r\n");
      } else {
        sprintf(line, "- Three phase system\r\n");
      }
      ui_start_up_add_line(line);
    }

    if (error) {

      ui_start_up_add_line_error("\r\nErrors: restarting...");

      ui_delay_untill_tapped(5000);
    }

  } while (error);

  ui_start_up_add_line("\r\nAll fine!");
}