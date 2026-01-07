#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"
#include "SmartEVSE.h"

Modbus mb = Modbus();

void smart_evse_init() {

  mb.begin(9600);
}

bool smart_evse_get_serial(int address, int *serial) {

  if (mb.readInputRegisters(address, 0x00, 5) == mb.ku8MBSuccess) {

    for(int i=0;i<5;i++) {
      serial[i] = mb.getResponseBuffer(i);
    }

    DEBUG_PRINTF("serial: %4x%4x%4x%4x%4x\r\n",serial[0],serial[1],serial[2],serial[3],serial[4]);

    return(true);

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }

  return(false);
}


bool smart_evse_get_fw_version(int address, int *version) {

  if (mb.readInputRegisters(address, 0x05, 1) == mb.ku8MBSuccess) {

    *version = mb.getResponseBuffer(0);

    DEBUG_PRINTF("fw version: %4x\r\n", *version);

    return(true);

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }

  return(false);
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

void smart_evse_set_phases(int address, bool isPhases3) {

  unsigned int value = (isPhases3?1:0);

  if (mb.writeSingleRegister(address, 0x204, value) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("wrote settings\r\n");

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}

bool smart_evse_is_32_amp(int address) {

  bool is32Amp = false;

  if (mb.readInputRegisters(address, 0x10c, 1) == mb.ku8MBSuccess) {

    // 0x10c options, bit 1 is 32/16a
    unsigned int value = mb.getResponseBuffer(0);

    is32Amp = (value & 0x02) > 0;

  } else {

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }

  return(is32Amp);
}

void smart_evse_get_charger_state(int address, ChargerState *charger_state) {

  charger_state->count++;

  if (mb.readInputRegisters(address, 0x103, 10) == mb.ku8MBSuccess) {
    
    // 0x103 state, error
    unsigned int value = mb.getResponseBuffer(0);
    charger_state->error = value >> 8;
    charger_state->state = value & 0xff;

    // 0x104 temperature
    charger_state->temperature = mb.getResponseBuffer(1);

    // 0x105 current l1 (1/256 a)
    charger_state->c[0] = (double)mb.getResponseBuffer(2)/256.;

    // 0x106 current l2 (1/256 a)
    charger_state->c[1] = (double)mb.getResponseBuffer(3)/256.;

    // 0x107 current l3 (1/256 a)
    charger_state->c[2] = (double)mb.getResponseBuffer(4)/256.;

    // 0x108 energy session (1/256 kwh)
    charger_state->kwh_session = (double)mb.getResponseBuffer(5)/256.;

    // 0x109 voltage l1 (1/256 v)
    charger_state->v[0] = (double)mb.getResponseBuffer(6)/256.;

    // 0x10a voltage l2 (1/256 v)
    charger_state->v[1] = (double)mb.getResponseBuffer(7)/256.;

    // 0x10b voltage l3 (1/256 v)
    charger_state->v[2] = (double)mb.getResponseBuffer(8)/256.;

    // 0x10c options, bit 1 is 32/16a

    // 0x10d energy total (low part, 1/256 kwh)
    // 0x10e energy total (high part, 1/256 kwh)
    charger_state->kwh_total = (double)mb.getResponseBuffer(10)/256. + (double)mb.getResponseBuffer(11)*256.;

    // time stamp;
    charger_state->ts = millis();
    charger_state->isNew = true;

  } else {

    charger_state->error_count++;

    DEBUG_PRINTF("failed to read over mb\r\n");

    if (address == 1) {
      DEBUG_PRINTF("try state change with button on the board\r\n");
    }
  }
}
