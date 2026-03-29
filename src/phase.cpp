#include "phase.h"
#include "modbus.h"

Phase phases[PHASE_COUNT]{};

// Delete later
float test_cntr = 0;

void read_voltages_n_currents() {
  uint8_t slave = DEFAULT_SOLIS_SLAVE_ADDR;
  uint8_t func_code = INV_OP_INF_FUNC_CODE;
  // read 6 regs in order VOLTAGE A, B, C, CURRENT A, B, C
  uint32_t data = (uint32_t)PHASE_A_VOLT_REG << 16 | 0x0006;
  send_modbus_req(slave, func_code, data);

  uint8_t resp[VOLT_CURR_EXPECTED_RES_LEN];
  if(read_modbus_resp(resp, VOLT_CURR_EXPECTED_RES_LEN)) {
    // print for now
    for(uint8_t i = 0; i < VOLT_CURR_EXPECTED_RES_LEN; ++i) {
      Serial.print((uint32_t)resp[i]);
      Serial.print(" ");
    }
    Serial.println("");
  }
  else {
    Serial.println("Serial2 read timed out");
  }
}

void send_phase_info(AsyncWebSocket& ws){
  char json[512];
  uint16_t pos = 0;
  pos += snprintf(json + pos, sizeof(json) - pos, "{");

  for(uint8_t i = 0; i < PHASE_COUNT; ++i) {
    pos += snprintf(json + pos, sizeof(json) - pos, "\"phase%u\":{\"voltage\":%f,\"current\":%f,\"power\":%f}", i, phases[i].voltage, phases[i].current, phases[i].power);
    if(i != PHASE_COUNT - 1) 
      pos += snprintf(json + pos, sizeof(json) - pos, ",");
  }

  snprintf(json + pos, sizeof(json) - pos, "}");
  ws.textAll(json);
}