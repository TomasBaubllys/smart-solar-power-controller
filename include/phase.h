#ifndef PHASE_H_INCLUDED
#define PHASE_H_INCLUDED

#include <cstdint>
#include <ESPAsyncWebServer.h>

constexpr uint8_t PHASE_COUNT = 3;

enum PHASE_NUM: uint8_t {
  A,
  B,
  C
};

struct Phase {
  float voltage;
  float current;
  float power;
};


extern Phase phases[PHASE_COUNT];

float read_phase_pwr(const PHASE_NUM phase_num);

float read_phase_voltage(const PHASE_NUM phase_num);

float read_phase_current(const PHASE_NUM phase_num);

void send_phase_info(AsyncWebSocket& ws);

#endif // PHASE_H_INCLUDED