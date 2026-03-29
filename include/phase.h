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

void read_voltages_n_currents();

void send_phase_info(AsyncWebSocket& ws);

#endif // PHASE_H_INCLUDED