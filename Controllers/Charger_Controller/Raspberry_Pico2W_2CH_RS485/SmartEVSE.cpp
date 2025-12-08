
#include <Arduino.h>
#include "ModBus.h"
#include "SmartEVSE.h"

extern Modbus mb;


void smart_evse_get_serial(int address) {

    if (mb.readInputRegisters(address, 0x00, 5) == mb.ku8MBSuccess) {
      
      Serial.printf("serial: %4x%4x%4x%4x%4x\r\n",
        mb.getResponseBuffer(0),
        mb.getResponseBuffer(1),
        mb.getResponseBuffer(2),
        mb.getResponseBuffer(3),
        mb.getResponseBuffer(4));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

void smart_evse_get_fw_version(int address) {

    if (mb.readInputRegisters(address, 0x05, 1) == mb.ku8MBSuccess) {
      
      Serial.printf("fw version: %4x\r\n", mb.getResponseBuffer(0));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

void smart_evse_persist_settings(int address) {

    if (mb.readInputRegisters(address, 0x0a, 1) == mb.ku8MBSuccess) {
      
      if (mb.getResponseBuffer(0) == 0) {

        Serial.printf("persisted\r\n");

      } else {

        Serial.printf("persist failed (did you unlock with magic?)\r\n");
      }

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}


void smart_evse_get_max_currents(int address) {

    if (mb.readInputRegisters(address, 0x102, 1) == mb.ku8MBSuccess) {
      
      int currents = mb.getResponseBuffer(0);

      int max_cable_current = currents & 0xff;
      int advertized_current = (currents >> 8) & 0xff;

      Serial.printf("max cable current %d\r\n", max_cable_current);
      Serial.printf("advertized current %d\r\n", advertized_current);

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}


void smart_evse_get_state(int address) {

    if (mb.readInputRegisters(address, 0x103, 1) == mb.ku8MBSuccess) {
      
      int states = mb.getResponseBuffer(0);

      int state = states & 0xff;
      int error = (states >> 8) & 0xff;

      switch(state) {
        case 0: Serial.printf("state undefined\r\n"); break;
        case 1: Serial.printf("state A\r\n"); break;
        case 2: Serial.printf("state B\r\n"); break;
        case 3: Serial.printf("state C\r\n"); break;
        case 4: Serial.printf("state D\r\n"); break;
        case 5: Serial.printf("state ERROR\r\n"); break;
        case 6: Serial.printf("hardware error\r\n"); break;
        default: Serial.printf("weird state (%d)\r\n", state); break;
      }

      // if(error == 0x00) {Serial.printf("no errors\r\n");}
      if(error&0x01) {Serial.printf("temperature too high\r\n");}
      if(error&0x02) {Serial.printf("stuck relay\r\n");}
      if(error&0x04) {Serial.printf("ground fault\r\n");}
      if(error&0x08) {Serial.printf("cp max too low\r\n");}
      if(error&0x10) {Serial.printf("cp min too high\r\n");}
      if(error&0x20) {Serial.printf("dc leak detected\r\n");}
      if(error&0x40) {Serial.printf("i2c initialize error\r\n");}
      if(error&0x80) {Serial.printf("i2c communication error\r\n");}

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}


void smart_evse_get_temperature(int address) {

    if (mb.readInputRegisters(address, 0x104, 1) == mb.ku8MBSuccess) {
      
      int temperature = mb.getResponseBuffer(0);

      Serial.printf("temperature %d\r\n", temperature);

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}


void smart_evse_get_current_draw(int address) {

    if (mb.readInputRegisters(address, 0x105, 3) == mb.ku8MBSuccess) {
      
      Serial.printf("current draw L1: %d\r\n", mb.getResponseBuffer(0));
      Serial.printf("current draw L2: %d\r\n", mb.getResponseBuffer(1));
      Serial.printf("current draw L3: %d\r\n", mb.getResponseBuffer(2));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}


void smart_evse_get_session_energy(int address) {

    if (mb.readInputRegisters(address, 0x108, 1) == mb.ku8MBSuccess) {
      
      Serial.printf("energy delivered in current session: %f kWh\r\n", (1./256.)*(double)mb.getResponseBuffer(0));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

void smart_evse_get_output_voltage(int address) {

    if (mb.readInputRegisters(address, 0x109, 3) == mb.ku8MBSuccess) {
      
      Serial.printf("voltage L1: %d V\r\n", mb.getResponseBuffer(0));
      Serial.printf("voltage L2: %d V\r\n", mb.getResponseBuffer(1));
      Serial.printf("voltage L3: %d V\r\n", mb.getResponseBuffer(2));

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

void smart_evse_set_max_current(int address, int max_current, int max_current_next_second) {

    if (mb.writeSingleRegister(address, 0x201, max_current + 256 * max_current_next_second) == mb.ku8MBSuccess) {
      
      Serial.printf("wrote max currents\r\n");

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

void smart_evse_set_magic(int address, int magic) {

    if (mb.writeSingleRegister(address, 0x202, magic) == mb.ku8MBSuccess) {
      
      Serial.printf("wrote magic\r\n");

    } else {

      Serial.printf("failed to read over mb\r\n");
    }
}

