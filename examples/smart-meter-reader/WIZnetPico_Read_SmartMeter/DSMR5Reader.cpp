#include <Arduino.h>
#include "DSMR5Reader.h"


#define ID_LENGTH 25
char id[ID_LENGTH];
bool dsmr_single_phase;
#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];
unsigned int line_buffer_index;

double tag_value[9];  // 3 x V, 6 x kW
int tags_read;
double dsmr_current[3];

void dsmr5reader_init() {

  // Serial1 is UART0 and default on TX=GP0, RX=GP1
  // the P1 signal is inverted; a larger receive buffer holds a whole telegram line burst
  Serial1.setInvertRX(true);
  Serial1.setFIFOSize(1024);
  Serial1.begin(115200);

  line_buffer_index = 0;
  tags_read = 0;
  dsmr_single_phase = true;
  dsmr_current[0] = 0;
  dsmr_current[1] = 0;
  dsmr_current[2] = 0;
}

// Wait for the start of a telegram ('/'), read it up to the end ('!') and check that it is DSMR 5.0
// (line 1-3:0.2.8(50)). Also finds out if the meter is single phase (no 1-0:72.7.0 line).
// Same check as in the smart-meter-load-balancing example.
int dsmr5reader_check(void) {

  unsigned long time_out_time = millis() + 2000UL;
  int c = 0;
  bool dsmr50 = false;

  dsmr_single_phase = true;

  do {
    if (Serial1.available()) {
      c = Serial1.read();
    }
  } while ((c != '/') && (millis() < time_out_time));

  if (c != '/') {

    // timeout
    return (DSMR_NO_DATA);
  }

  int n = Serial1.readBytesUntil('\r', id, ID_LENGTH - 1);
  id[n] = 0;
  // read \n
  Serial1.read();

  do {

    // read lines till got: 1-3:0.2.8(50)
    n = Serial1.readBytesUntil('\r', line_buffer, LINE_BUFFER_SIZE - 1);
    line_buffer[n] = 0;
    // read \n
    Serial1.read();

    if (!strncmp(line_buffer, "1-0:72.7.0", 10)) {
      dsmr_single_phase = false;
    }

    if (!strncmp(line_buffer, "1-3:0.2.8(", 10)) {
      if ((line_buffer[10] == '5') && (line_buffer[11] == '0')) {
        dsmr50 = true;
      }
    }

    if (line_buffer[0] == '!') {

      // end of telegram
      return (dsmr50 ? DSMR_OK : DSMR_NOT_DSMR_50);
    }

  } while (millis() < time_out_time);

  return (DSMR_NOT_DSMR);
}


static void process_tag_line() {

  // 1-0:32.7.0(229.5*V)
  // 1-0:21.7.0(00.106*kW)
  // ^^^^ = "1-0:"
  //     ^^ = sub_tag
  //       ^^^^ = ".7.0"
  //                 ^ = end of V value, replace by 0
  //                  ^ = end of kW value, replace by 0
  // 012345678901234567

  if (strncmp(line_buffer, "1-0:", 4) || strncmp(&line_buffer[6], ".7.0", 4)) {
    return;
  }

  int sub_tag = ((int)line_buffer[4] << 8) + line_buffer[5];
  int tag = -1;
  int end = 17;

  switch (sub_tag) {
    case ((int)'3' << 8) + '2': tag = 0; end = 16; break;  // voltage L1
    case ((int)'5' << 8) + '2': tag = 1; end = 16; break;  // voltage L2
    case ((int)'7' << 8) + '2': tag = 2; end = 16; break;  // voltage L3
    case ((int)'2' << 8) + '1': tag = 3; break;            // power delivered L1
    case ((int)'4' << 8) + '1': tag = 4; break;            // power delivered L2
    case ((int)'6' << 8) + '1': tag = 5; break;            // power delivered L3
    case ((int)'2' << 8) + '2': tag = 6; break;            // power returned L1
    case ((int)'4' << 8) + '2': tag = 7; break;            // power returned L2
    case ((int)'6' << 8) + '2': tag = 8; break;            // power returned L3
  }

  if (tag >= 0) {
    line_buffer[end] = 0;
    tag_value[tag] = atof(&line_buffer[11]);
    tags_read++;
  }
}

// Call often. Returns 1 when a complete telegram was read and dsmr_current[] is updated.
int dsmr5reader_process_line() {

  int new_current = 0;

  while (Serial1.available()) {

    int c = Serial1.read();

    if (c <= 0) {
      continue;
    }

    if (c != '\n') {

      if (line_buffer_index < LINE_BUFFER_SIZE - 1) {
        line_buffer[line_buffer_index++] = (char)c;
      }
      continue;
    }

    // complete line
    line_buffer[line_buffer_index] = 0;

    if (line_buffer[0] == '/') {

      // start of telegram
      tags_read = 0;
      for (int tag = 0; tag < 9; tag++) {
        tag_value[tag] = 0;
      }

    } else if (line_buffer[0] == '!') {

      // end of telegram
      if (tags_read == (dsmr_single_phase ? 3 : 9)) {

        for (int phase = 0; phase < 3; phase++) {

          // a single-phase meter only sends L1; no voltage means no current (no division by zero)
          if (tag_value[phase] > 1.0) {
            dsmr_current[phase] = 1000. * (tag_value[3 + phase] - tag_value[6 + phase]) / tag_value[phase];
          } else {
            dsmr_current[phase] = 0;
          }
        }

        new_current = 1;
      }

    } else if (line_buffer_index > 10) {

      process_tag_line();
    }

    line_buffer_index = 0;

    if (new_current) {
      break;
    }
  }

  return (new_current);
}
