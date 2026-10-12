#include <Arduino.h>
#include "Board.h"
#include "ModBus.h"
#include "SmartEVSE.h"

// Modbus helpers for the EdgeTech charge controller (EVSE-C-3x32A).
// Register map, scaling and state numbering: docs/en/modbus-registers.md
//   https://github.com/edge-tech-eu/SmartEVSE/blob/main/docs/en/modbus-registers.md
// Currents, voltages and energy are in 1/256 units: divide by 256.
// State (0x0103 low byte) is 1-indexed: 1 = A, 2 = B, 3 = C, 4 = D.

Modbus mb = Modbus();

static void report_no_answer(int address) {

  DEBUG_PRINTF("failed to read over mb (address %d)\r\n", address);

  if (address == 1) {
    DEBUG_PRINTF("no answer: at address 1 the charger only answers after a state change\r\n");
    DEBUG_PRINTF("(press the button on the board or plug in a car), or give it address 2 or higher via register 0x0200\r\n");
  }
}

void smart_evse_init() {

  mb.begin(9600);
}

void smart_evse_get_serial(int address) {

  if (mb.readInputRegisters(address, 0x00, 5) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("serial: %04x%04x%04x%04x%04x\r\n",
                  mb.getResponseBuffer(0),
                  mb.getResponseBuffer(1),
                  mb.getResponseBuffer(2),
                  mb.getResponseBuffer(3),
                  mb.getResponseBuffer(4));

  } else {

    report_no_answer(address);
  }
}


void smart_evse_get_fw_version(int address) {

  if (mb.readInputRegisters(address, 0x05, 1) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("fw version: %4x\r\n", mb.getResponseBuffer(0));

  } else {

    report_no_answer(address);
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

    report_no_answer(address);
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

    report_no_answer(address);
  }
}


void smart_evse_get_state(int address) {

  if (mb.readInputRegisters(address, 0x103, 1) == mb.ku8MBSuccess) {

    int states = mb.getResponseBuffer(0);

    int state = states & 0xff;
    int error = (states >> 8) & 0xff;

    // 0x0103 low byte, 1-indexed (see docs/en/modbus-registers.md)
    switch (state) {
      case 1: DEBUG_PRINTF("state A, no car connected\r\n"); break;
      case 2: DEBUG_PRINTF("state B, car connected, not charging\r\n"); break;
      case 3: DEBUG_PRINTF("state C, car connected, charging\r\n"); break;
      case 4: DEBUG_PRINTF("state D, car connected, charging, ventilation required\r\n"); break;
      default: DEBUG_PRINTF("unexpected state (%d)\r\n", state); break;
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

    report_no_answer(address);
  }
}


void smart_evse_get_temperature(int address) {

  if (mb.readInputRegisters(address, 0x104, 1) == mb.ku8MBSuccess) {

    int temperature = (int16_t)mb.getResponseBuffer(0);

    DEBUG_PRINTF("board temperature %d\r\n", temperature);

  } else {

    report_no_answer(address);
  }
}


void smart_evse_get_current_draw(int address) {

  // 0x105..0x107, 1/256 A
  if (mb.readInputRegisters(address, 0x105, 3) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("current draw L1: %.2f A\r\n", (double)mb.getResponseBuffer(0) / 256.);
    DEBUG_PRINTF("current draw L2: %.2f A\r\n", (double)mb.getResponseBuffer(1) / 256.);
    DEBUG_PRINTF("current draw L3: %.2f A\r\n", (double)mb.getResponseBuffer(2) / 256.);

  } else {

    report_no_answer(address);
  }
}


void smart_evse_get_session_energy(int address) {

  // 0x108, 1/256 kWh
  if (mb.readInputRegisters(address, 0x108, 1) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("energy delivered in current session: %f kWh\r\n", (double)mb.getResponseBuffer(0) / 256.);

  } else {

    report_no_answer(address);
  }
}

void smart_evse_get_output_voltage(int address) {

  // 0x109..0x10b, 1/256 V
  if (mb.readInputRegisters(address, 0x109, 3) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("voltage L1: %.1f V\r\n", (double)mb.getResponseBuffer(0) / 256.);
    DEBUG_PRINTF("voltage L2: %.1f V\r\n", (double)mb.getResponseBuffer(1) / 256.);
    DEBUG_PRINTF("voltage L3: %.1f V\r\n", (double)mb.getResponseBuffer(2) / 256.);

  } else {

    report_no_answer(address);
  }
}

// 0x0201: low byte = RemoteMaxCurrent (A, 0 stops charging), high byte = RemoteMaxCurrentNextSecond.
// Write at least once per second; after two missed writes the charger falls back to its static limit.
void smart_evse_set_max_current(int address, int max_current, int max_current_next_second) {

  uint16_t value = (uint16_t)((max_current & 0xff) | ((max_current_next_second & 0xff) << 8));

  if (mb.writeSingleRegister(address, 0x201, value) != mb.ku8MBSuccess) {

    report_no_answer(address);
  }
}

void smart_evse_set_magic(int address, int magic) {

  if (mb.writeSingleRegister(address, 0x202, magic) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("wrote magic\r\n");

  } else {

    report_no_answer(address);
  }
}

// 0x0204 settings bitfield: bit 0 = PHASES (0 = 1-phase, 1 = 3-phase).
// Read-modify-write, so the other bits (cable lock, DC leakage, CP disconnect) are kept.
bool smart_evse_set_phases(int address, bool isPhases3) {

  if (mb.readHoldingRegisters(address, 0x204, 1) != mb.ku8MBSuccess) {

    DEBUG_PRINTF("could not read settings, phases not changed\r\n");
    report_no_answer(address);
    return(false);
  }

  uint16_t value = mb.getResponseBuffer(0);

  value = (uint16_t)((value & ~0x0001) | (isPhases3 ? 0x0001 : 0x0000));

  if (mb.writeSingleRegister(address, 0x204, value) == mb.ku8MBSuccess) {

    DEBUG_PRINTF("wrote settings 0x%04x\r\n", value);
    return(true);

  } else {

    report_no_answer(address);
    return(false);
  }
}

bool smart_evse_is_32_amp(int address) {

  bool is32Amp = false;

  if (mb.readInputRegisters(address, 0x10c, 1) == mb.ku8MBSuccess) {

    // 0x10c options, bit 1 is 32/16a
    unsigned int value = mb.getResponseBuffer(0);

    is32Amp = (value & 0x02) > 0;

  } else {

    report_no_answer(address);
  }

  return(is32Amp);
}

void smart_evse_get_charger_state(int address, ChargerState *charger_state) {

  charger_state->count++;

  // read 0x102 up to and including 0x10e (13 registers)
  if (mb.readInputRegisters(address, 0x102, 13) == mb.ku8MBSuccess) {

    // 0x102 low byte: max cable current (A), high byte: advertised current (A)
    charger_state->cable_current = mb.getResponseBuffer(0) & 0xff;

    // 0x103 low byte: state (1 = A, 2 = B, 3 = C, 4 = D), high byte: error bits
    unsigned int value = mb.getResponseBuffer(1);
    charger_state->error = value >> 8;
    charger_state->state = value & 0xff;

    // 0x104 temperature
    charger_state->temperature = mb.getResponseBuffer(2);

    // 0x105 current l1 (1/256 a)
    charger_state->c[0] = (double)mb.getResponseBuffer(3)/256.;

    // 0x106 current l2 (1/256 a)
    charger_state->c[1] = (double)mb.getResponseBuffer(4)/256.;

    // 0x107 current l3 (1/256 a)
    charger_state->c[2] = (double)mb.getResponseBuffer(5)/256.;

    // 0x108 energy session (1/256 kwh)
    charger_state->kwh_session = (double)mb.getResponseBuffer(6)/256.;

    // 0x109 voltage l1 (1/256 v)
    charger_state->v[0] = (double)mb.getResponseBuffer(7)/256.;

    // 0x10a voltage l2 (1/256 v)
    charger_state->v[1] = (double)mb.getResponseBuffer(8)/256.;

    // 0x10b voltage l3 (1/256 v)
    charger_state->v[2] = (double)mb.getResponseBuffer(9)/256.;

    // 0x10c options, bit 1 is 32/16a (buffer 10)

    // 0x10d energy total, low word; 0x10e energy total, high word (32 bit, 1/256 kwh)
    uint32_t energy = ((uint32_t)mb.getResponseBuffer(12) << 16) | (uint32_t)mb.getResponseBuffer(11);
    charger_state->kwh_total = (double)energy / 256.;

    // time stamp;
    charger_state->ts = millis();
    charger_state->isNew = true;

  } else {

    charger_state->error_count++;

    report_no_answer(address);
  }
}
