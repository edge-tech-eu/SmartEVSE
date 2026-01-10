#include <Arduino.h>
#include "Board.h"
#include "DSMR5Reader.h"


#define ID_LENGTH 25
char dsmr_id[ID_LENGTH];
bool dsmr_single_phase;
#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];
unsigned int line_buffer_index;

double tag_value[9];  // 3 x V, 6 x kW
double dsmr_current[3];

int max_a = 0;
int last_max_a = 0;
bool last_was_newline;

void dsmr5reader_init() {

#ifdef ARDUINO_ARCH_RP2040

#define UART_READ_BUFFER_SIZE 32
#define UART_READ_BUFFER_SIZE_WARNING 30

  // Serial1 is UART0 and default on TX=GP0, RX=GP1
  Serial1.setInvertRX(true);
  Serial1.begin(115200);

#elif ARDUINO_M5STACK_CORES3

#define UART_READ_BUFFER_SIZE 300
#define UART_READ_BUFFER_SIZE_WARNING 275

  Serial2.setRxBufferSize(UART_READ_BUFFER_SIZE);

  Serial1.begin(115200, SERIAL_8N1, 2, 1, true);
  //Serial1.begin(115200, SERIAL_8N1, 2, 1);

#endif

  line_buffer_index = 0;
  last_was_newline = false;
}

int dsmr5reader_check(void) {

  unsigned long time_out_time = millis() + 2000UL;
  int c;
  int count = 0;
  bool dsmr50 = false;

  dsmr_single_phase = true;

  do {
    if (Serial1.available()) {
      c = Serial1.read();
      if (c > 0) {
        count++;
      }
    }
  } while ((c != '/') && (millis() < time_out_time));

  if (c != '/') {

    Serial.printf("data read so far: %d\r\n", count);
    return (DSMR_NO_DATA);
  }


  int n = Serial1.readBytesUntil('\r', dsmr_id, ID_LENGTH);
  dsmr_id[n] = 0;
  Serial1.read();

  do {

    // read lines till got: 1-3:0.2.8(50)
    int n = Serial1.readBytesUntil('\r', line_buffer, LINE_BUFFER_SIZE);
    line_buffer[n] = 0;
    // read \n
    Serial1.read();

    if (!strncmp(line_buffer, "1-0:72.7.0", 10)) {
      dsmr_single_phase = false;
    }

    if (!strncmp(line_buffer, "1-3:0.2.8(", 10)) {

      if ((line_buffer[10] == '5') && (line_buffer[11] == '0')) {
        
        dsmr50 = true;
        //return (DSMR_OK);

      } else {

        //return (DSMR_NOT_DSMR_50);
      }
    }

    if(line_buffer[0] == '!') {
      return(dsmr50?DSMR_OK:DSMR_NOT_DSMR_50);
    }

  } while (millis() < time_out_time);

  return (DSMR_NOT_DSMR);
}


bool dsmr5reader_process_line() {

  int c;

  int availables = Serial1.available();

  if (availables) {

    if (availables > UART_READ_BUFFER_SIZE_WARNING) {
      Serial.printf("UART buffer might be too small! (available: %d, warning: %d, size: %d)", availables, UART_READ_BUFFER_SIZE_WARNING, UART_READ_BUFFER_SIZE);
    }

    // keep reading till buffer empty

    while (Serial1.available()) {

      // read line till new line

      if (!last_was_newline) {

        while (Serial1.available()) {

          c = Serial1.read();

          if (c > 0) {

            line_buffer[line_buffer_index++] = c;

            if (line_buffer_index > LINE_BUFFER_SIZE) {
              line_buffer_index = 0;
            }

            if (c == '\n') {
              last_was_newline = true;
              break;
            }
          }

          last_was_newline = false;
        }
      }

      // can we process a line?

      if (last_was_newline) {

        last_was_newline = false;

        // Serial.printf("got %c , %d and %d\r\n", line_buffer[0], (int)line_buffer_index, (int)line_buffer[line_buffer_index - 1]);

        if (line_buffer[0] == 47) {

          // start of telegram

          // Serial.printf("\r\ns\r\n");

          for (int tag = 0; tag < 9; tag++) {
            tag_value[tag] = -1;
          }

        } else if (line_buffer[0] == '!') {

          // end of telegram

          // for (int tag = 0; tag < 8; tag++) {
          //  Serial.printf("%d: %f\r\n", tag, tag_value[tag]);
          // }

          for (int phase = 0; phase < 3; phase++) {
            dsmr_current[phase] = 1000. * (tag_value[3 + phase] - tag_value[6 + phase]) / tag_value[phase];
          }

          // done processing line and telegram

          line_buffer_index = 0;

          return (true);

        } else if (line_buffer_index > 10) {

          // voltage & power tags?

          if (!strncmp(line_buffer, "1-0:", 4)) {

            // 1-0:32.7.0(229.5*V)
            // 1-0:21.7.0(00.106*kW)
            // ^^^^ = "1-0:"
            //     ^^ = sub_tag
            //        ^^^^ = ".7.0"
            //                 ^ = end of V value, replace by 0
            //                  ^ = end of kW value, replace by 0
            // 012345678901234567

            // 1-0:32.7.0(229.0*V)
            // 1-0:21.7.0(00.083*kW)
            // 1-0:22.7.0(00.000*kW)

            if (!strncmp(&line_buffer[6], ".7.0", 4)) {

              int sub_tag = ((int)line_buffer[4] << 8) + line_buffer[5];

              switch (sub_tag) {
                case ((int)'3' << 8) + '2':
                  line_buffer[16] = 0;
                  tag_value[0] = atof(&line_buffer[11]);
                  break;
                case ((int)'5' << 8) + '2':
                  line_buffer[16] = 0;
                  tag_value[1] = atof(&line_buffer[11]);
                  break;
                case ((int)'7' << 8) + '2':
                  line_buffer[16] = 0;
                  tag_value[2] = atof(&line_buffer[11]);
                  break;
                case ((int)'2' << 8) + '1':
                  line_buffer[17] = 0;
                  tag_value[3] = atof(&line_buffer[11]);
                  break;
                case ((int)'4' << 8) + '1':
                  line_buffer[17] = 0;
                  tag_value[4] = atof(&line_buffer[11]);
                  break;
                case ((int)'6' << 8) + '1':
                  line_buffer[17] = 0;
                  tag_value[5] = atof(&line_buffer[11]);
                  break;
                case ((int)'2' << 8) + '2':
                  line_buffer[17] = 0;
                  tag_value[6] = atof(&line_buffer[11]);
                  break;
                case ((int)'4' << 8) + '2':
                  line_buffer[17] = 0;
                  tag_value[7] = atof(&line_buffer[11]);
                  break;
                case ((int)'6' << 8) + '2':
                  line_buffer[17] = 0;
                  tag_value[8] = atof(&line_buffer[11]);
                  break;
              }
            }
          }
        }

        // done processing line

        line_buffer_index = 0;

      }  // if new_line
    }    // while available
  }      // if available

  return (false);
}
