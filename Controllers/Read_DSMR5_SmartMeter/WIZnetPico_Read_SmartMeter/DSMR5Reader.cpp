#include <Arduino.h>
#include "DSMR5Reader.h"


#define ID_LENGTH 25
char id[ID_LENGTH];
#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];
unsigned int line_buffer_index;

double tag_value[9];  // 3 x V, 6 x kW
double dsmr_current[3];

void dsmr5reader_init() {

  // Serial1 is UART0 and default on TX=GP0, RX=GP1
  Serial1.setInvertRX(true);
  Serial1.begin(115200);

  line_buffer_index = 0;
}

int dsmr5reader_check(void) {

  unsigned long time_out_time = millis() + 2000UL;
  int c;

  do {

    // find for '\n'
    do {
      if (Serial1.available()) {
        c = Serial1.read();
      }
    } while ((c != '\n') && (millis() < time_out_time));

    if(c!='\n') {
      
      // timeout
      return(DSMR_NO_DATA);
    }

    // first char is '/'
    c = Serial1.read();

  } while ((c != '/') && (millis() < time_out_time));

  if(c!='/') {
      
      // timeout
      return(DSMR_INCORRECT_DATA);
  }

  int n = Serial1.readBytesUntil('\r',id, ID_LENGTH);
  id[n] = 0;
  Serial1.read();

  int compare;
  do {
  
    // read lines till got: 1-3:0.2.8(50)
    int n = Serial1.readBytesUntil('\r',line_buffer, LINE_BUFFER_SIZE);
    line_buffer[n] = 0;
    Serial1.read();
    compare = strncmp(line_buffer, "1-3:0.2.8(", 10);

  } while((!compare) && (millis() < time_out_time));

  if(!compare) {
      
      // timeout
      return(DSMR_NOT_DSMR);
    }
  
  if((line_buffer[11]!='5')||line_buffer[11]!='0') {

    // no dsrm 5 meter
      return(DSMR_NOT_DSMR_50);
  }

  return (DSMR_OK);
}


int dsmr5reader_process_line() {

  int new_current = 0;
  int c;

  if (Serial1.available()) {

    // receive buffer is only 32 bytes long, so read all we can

    while (Serial1.available()) {

      c = Serial1.read();
      line_buffer[line_buffer_index++] = c;

      if (line_buffer_index > LINE_BUFFER_SIZE) {
        line_buffer_index = 0;
      }

      if (c != '\n') {
        break;
      }
    }
  }

  if (c != '\n') {
    return (new_current);
  }

  if (line_buffer[0] == '/') {

    // start of telegram

    for (int tag = 0; tag < 9; tag++) {
      tag_value[tag] = -1;
    }

  } else if (line_buffer[0] == '!') {

    // end of telegram

    for (int phase = 0; phase < 3; phase++) {
      dsmr_current[phase] = 1000. * (tag_value[3 + phase] - tag_value[6 + phase]) / tag_value[phase];
    }

    // Serial.printf("Current: %f, %f, %f\r\n",dsmr_current[0],dsmr_current[1],dsmr_current[2]);

    new_current = 1;

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

  line_buffer_index = 0;

  return (new_current);
}
