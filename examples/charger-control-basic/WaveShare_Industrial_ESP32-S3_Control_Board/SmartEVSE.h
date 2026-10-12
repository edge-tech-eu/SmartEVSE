#ifndef SMARTEVSE_h
#define SMARTEVSE_h

// Values as read from the EdgeTech charge controller, see docs/en/modbus-registers.md
// state: 1 = A, 2 = B, 3 = C, 4 = D (0x0103 low byte). Currents in A, voltages in V, energy in kWh.

typedef struct {
  unsigned long ts;
  bool isNew;
  unsigned char cable_current;   // 0x0102 low byte, A
  double c[3];
  double v[3];
  double kwh_session;
  double kwh_total;
  unsigned char state;
  unsigned char error;
  unsigned char temperature;
  unsigned int error_count;
  unsigned int count;
} ChargerState;

void smart_evse_init();
bool smart_evse_set_phases(int address, bool isPhases3);
bool smart_evse_is_32_amp(int address);
void smart_evse_get_charger_state(int address, ChargerState *charger_state);
void smart_evse_get_serial(int address);
void smart_evse_get_fw_version(int address);
void smart_evse_persist_settings(int address);
void smart_evse_get_max_currents(int address);
void smart_evse_get_state(int address);
void smart_evse_get_temperature(int address);
void smart_evse_get_current_draw(int address);
void smart_evse_get_session_energy(int address);
void smart_evse_get_output_voltage(int address);
void smart_evse_set_max_current(int address, int max_current, int max_current_next_second);
void smart_evse_set_magic(int address, int magic);

#endif