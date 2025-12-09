#include <Arduino.h>
#include "DebugPrintf.h"
#include "ModBus.h"
#include "SmartEVSE.h"

extern Modbus mb;

void smart_evse_get_serial(int address) {

  if (mb.readInputRegisters(address, 0x00, 5) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("serial: %4x%4x%4x%4x%4x\r\n",
                  mb.getResponseBuffer(0),
                  mb.getResponseBuffer(1),
                  mb.getResponseBuffer(2),
                  mb.getResponseBuffer(3),
                  mb.getResponseBuffer(4));

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_fw_version(int address) {

  if (mb.readInputRegisters(address, 0x05, 1) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("fw version: %4x\r\n", mb.getResponseBuffer(0));

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_persist_settings(int address) {

  if (mb.readInputRegisters(address, 0x0a, 1) == mb.ku8MBSuccess) {

    if (mb.getResponseBuffer(0) == 0) {

      DEBUG_PRINTF("persisted\r\n");

    } else {

      DEBUG_PRINTF("persist failed (did you unlock with magic?)\r\n");
    }

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_max_currents(int address) {

  if (mb.readInputRegisters(address, 0x102, 1) == mb.ku8MBSuccess) {

    int currents = mb.getResponseBuffer(0);

    int max_cable_current = currents & 0xff;
    int advertized_current = (currents >> 8) & 0xff;

    DEBUG_PRINTF("max cable current %d\r\n", max_cable_current);
    DEBUG_PRINTF("advertized current %d\r\n", advertized_current);

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_state(int address) {

  if (mb.readInputRegisters(address, 0x103, 1) == mb.ku8MBSuccess) {

    int states = mb.getResponseBuffer(0);

    int state = states & 0xff;
    int error = (states >> 8) & 0xff;

    switch (state) {
      case 0: DEBUG_PRINTF("state undefined\r\n"); break;
      case 1: DEBUG_PRINTF("state A, no car connected\r\n"); break;
      case 2: DEBUG_PRINTF("state B, car connected, not charging\r\n"); break;
      case 3: DEBUG_PRINTF("state C, car connected, charging\r\n"); break;
      case 4: DEBUG_PRINTF("state D, car connected, charging, ventilation requited\r\n"); break;
      case 5: DEBUG_PRINTF("state ERROR\r\n"); break;
      case 6: DEBUG_PRINTF("hardware error\r\n"); break;
      default: DEBUG_PRINTF("weird state (%d)\r\n", state); break;
    }

    // if(error == 0x00) {DEBUG_PRINTF("no errors\r\n");}
    if (error & 0x01) { DEBUG_PRINTF("temperature too high\r\n"); }
    if (error & 0x02) { DEBUG_PRINTF("stuck relay\r\n"); }
    if (error & 0x04) { DEBUG_PRINTF("ground fault\r\n"); }
    if (error & 0x08) { DEBUG_PRINTF("cp max too low\r\n"); }
    if (error & 0x10) { DEBUG_PRINTF("cp min too high\r\n"); }
    if (error & 0x20) { DEBUG_PRINTF("dc leak detected\r\n"); }
    if (error & 0x40) { DEBUG_PRINTF("i2c initialize error\r\n"); }
    if (error & 0x80) { DEBUG_PRINTF("i2c communication error\r\n"); }

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_temperature(int address) {

  if (mb.readInputRegisters(address, 0x104, 1) == mb.ku8MBSuccess) {

    int temperature = mb.getResponseBuffer(0);

    DEBUG_PRINTF("board temperature %d\r\n", temperature);

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_current_draw(int address) {

  if (mb.readInputRegisters(address, 0x105, 3) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("current draw L1: %d\r\n", mb.getResponseBuffer(0));
    DEBUG_PRINTF("current draw L2: %d\r\n", mb.getResponseBuffer(1));
    DEBUG_PRINTF("current draw L3: %d\r\n", mb.getResponseBuffer(2));

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}


void smart_evse_get_session_energy(int address) {

  if (mb.readInputRegisters(address, 0x108, 1) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("energy delivered in current session: %f kWh\r\n", (1. / 256.) * (double)mb.getResponseBuffer(0));

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}

void smart_evse_get_output_voltage(int address) {

  if (mb.readInputRegisters(address, 0x109, 3) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("voltage L1: %d V\r\n", mb.getResponseBuffer(0));
    DEBUG_PRINTF("voltage L2: %d V\r\n", mb.getResponseBuffer(1));
    DEBUG_PRINTF("voltage L3: %d V\r\n", mb.getResponseBuffer(2));

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}

void smart_evse_set_max_current(int address, int max_current, int max_current_next_second) {

  if (mb.writeSingleRegister(address, 0x201, max_current + 256 * max_current_next_second) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("wrote max currents\r\n");

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}

void smart_evse_set_magic(int address, int magic) {

  if (mb.writeSingleRegister(address, 0x202, magic) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("wrote magic\r\n");

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}
