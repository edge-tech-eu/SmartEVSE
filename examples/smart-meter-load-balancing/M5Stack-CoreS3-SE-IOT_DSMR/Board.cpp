#include <M5Unified.h>
#include <M5GFX.h>
#include <SPI.h>
#include <M5Module_LAN.h>
#include "Board.h"
#include "SmartEVSE.h"
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

void board_system_check(int modbus_address) {

  bool error;
  int serial[5];
  int version;
  char line[80];

  do {

    error = false;

    ui_clear();

    ui_start_up();

    ui_start_up_add_line("\r\n- Connecting to charger: ");

    smart_evse_init();

    if (!smart_evse_get_serial(modbus_address, serial)) {

      ui_start_up_add_line_error("not found!\r\n  check wires and address\r\n  change state if at address 1\r\n");
      board_max_current = 0;
      error = true;

    } else {

      ui_start_up_add_line("found!\r\n");
      sprintf(line, "- Serial number: %04x%04x%04x%04x%04x\r\n", serial[0], serial[1], serial[2], serial[3], serial[4]);
      smart_evse_get_fw_version(modbus_address, &version);
      ui_start_up_add_line(line);
      sprintf(line, "- Firmware version: %d.%d.%d\r\n", version >> 12, (version >> 8) & 0x0f, version & 0xff);
      ui_start_up_add_line(line);
      board_max_current = 16;

      if (smart_evse_is_32_amp(modbus_address)) {
        sprintf(line, "- Board max current: %d\r\n", board_max_current);
        ui_start_up_add_line(line);
      }
    }

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

    ui_start_up_add_line("- Charger comm test: ");
    int errors = 0;
    for (int i = 0; i < 10; i++) {
      bool found_error = false;
      for (int j = 0; j < 10; j++) {
        if (!(smart_evse_get_serial(modbus_address, serial) && smart_evse_get_fw_version(modbus_address, &version))) {
          errors++;
          found_error = true;
        }
      }
      if (found_error) {
        ui_start_up_add_line_error("x");
      } else {
        ui_start_up_add_line(".");
      }
    }
    int quality = 100 - errors;
    sprintf(line, " %d%%\r\n", quality);
    if (quality < 100) {
      ui_start_up_add_line_error(line);
      error = true;
    } else {
      ui_start_up_add_line(line);
    }

    if (error) {

      ui_start_up_add_line_error("\r\nErrors: restarting...");

      ui_delay_untill_tapped(5000);
    }

  } while (error);

  ui_start_up_add_line("\r\nAll fine!");
}