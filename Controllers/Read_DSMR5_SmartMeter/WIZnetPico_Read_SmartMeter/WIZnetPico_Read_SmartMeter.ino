
// board:     W55RP2.0-EVB-Pico
// datasheet: https://docs.wiznet.io/Product/Chip/MCU/W55RP20/w55rp20-evb-pico
// bsp:       https://github.com/earlephilhower/arduino-pico
// doc:       https://arduino-pico.readthedocs.io/en/latest/
// pinout:    https://docs.wiznet.io/Product/Chip/MCU/W55RP20/w55rp20-evb-pico#hardware-specification

#include <W55RP20lwIP.h>


#define line_buffer_size 256
char line_buffer[line_buffer_size];
unsigned int line_buffer_index;
double tag_value[9]; // 3 x V, 6 x kW
double current[3];

void process_line_init() {

  line_buffer_index = 0;
}

void process_line() {

  int c;

  if(Serial1.available()) {

    // receive buffer is only 32 bytes long, so read all we can

    while(Serial1.available()) {

      c = Serial1.read();
      line_buffer[line_buffer_index++] = c;

      if(line_buffer_index > line_buffer_size) {
        line_buffer_index = 0;
      }

      if(c != '\n') {
        break;
      }
    }
  }

  if(c != '\n') {
    return;
  }

  if (line_buffer[0] == '/') {

    // start of telegram

    for(int tag=0; tag<9; tag++) {
      tag_value[tag] = -1;
    }

  } else if (line_buffer[0] == '!') {

    // end of telegram

    for(int phase=0; phase<3; phase++) {
      current[phase] = 1000.*(tag_value[3+phase]-tag_value[6+phase])/tag_value[phase];
    }

    Serial.printf("Current: %f, %f, %f\r\n",current[0],current[1],current[2]);

  } else if(line_buffer_index > 10) {
    
    // voltage & power tags?

    
    if(!strncmp(line_buffer,"1-0:",4)) {

      // 1-0:32.7.0(229.5*V)
      // 1-0:21.7.0(00.106*kW)
      // ^^^^ = "1-0:"
      //     ^^ = sub_tag
      //        ^^^^ = ".7.0"  
      //                 ^ = end of V value, replace by 0
      //                  ^ = end of kW value, replace by 0
      // 012345678901234567

      if(!strncmp(&line_buffer[6],".7.0",4)) {

        int sub_tag = ((int)line_buffer[4]<<8) + line_buffer[5];

        switch(sub_tag) {
          case ((int)'3'<<8)+'2':
            line_buffer[16] = 0;
            tag_value[0] = atof(&line_buffer[11]);
            break;
          case ((int)'5'<<8)+'2':
            line_buffer[16] = 0;
            tag_value[1] = atof(&line_buffer[11]);
            break;
          case ((int)'7'<<8)+'2':
            line_buffer[16] = 0;
            tag_value[2] = atof(&line_buffer[11]);
            break;
          case ((int)'2'<<8)+'1':
            line_buffer[17] = 0;
            tag_value[3] = atof(&line_buffer[11]);
            break;
          case ((int)'4'<<8)+'1':
            line_buffer[17] = 0;
            tag_value[4] = atof(&line_buffer[11]);
            break;
          case ((int)'6'<<8)+'1':
            line_buffer[17] = 0;
            tag_value[5] = atof(&line_buffer[11]);
            break;
          case ((int)'2'<<8)+'2':
            line_buffer[17] = 0;
            tag_value[6] = atof(&line_buffer[11]);
            break;
          case ((int)'4'<<8)+'2':
            line_buffer[17] = 0;
            tag_value[7] = atof(&line_buffer[11]);
            break;
          case ((int)'6'<<8)+'2':
            line_buffer[17] = 0;
            tag_value[8] = atof(&line_buffer[11]);
            break;
        }
      }
    }
  }

  line_buffer_index = 0;
}


// ChipSelect:
Wiznet55rp20lwIP eth(20);

void setup() {
  
  Serial.begin(115200);
  delay(10000);

  Serial.printf("\r\n\r\nTesting WIZnet W55RP20 EVB\r\nTesting SmartMeter reading...\r\n\r\n");

  pinMode(LED_BUILTIN, OUTPUT);


  if(!eth.begin()) {
    Serial.printf("No wired Ethernet hardware detected. Check pinouts, wiring.\r\n");
    while (1) {
      delay(1000);
    }
  }

  while (!eth.connected()) {
      Serial.print(".");
      delay(500);
  }
  Serial.println();

  Serial.print("IP address: ");
  Serial.println(eth.localIP());

  // Serial1 is UART0 and default on TX=GP0, RX=GP1
  Serial1.setInvertRX(true);
  Serial1.begin(115200);

  process_line_init();
}




void loop() {
  
  process_line();
}



/*
void loop() {
  static bool wait = false;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from RP2040");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  if (wait) {
    delay(300000);  // execute once every 5 minutes, don't flood remote service
  }
  wait = true;
}
*/