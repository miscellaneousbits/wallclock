#pragma once

#include "main.h"

void clock_command(struct server* server, const uint8_t* cmd, uint8_t len);
uint64_t getime(void);
int get_clock_face_time(char* ts, uint64_t* t);

extern uint64_t gFaceTime;
extern volatile time_t gLastPollTime;
extern volatile uint16_t gPollInterval;
extern volatile int64_t gDelta;
extern volatile uint16_t gVbat;
extern volatile uint16_t gPollCount;
extern volatile uint16_t gMatch;
extern volatile uint16_t gTimeouts;
