#ifndef BEEP_H
#define BEEP_H

#include <stdint.h>

void beep(uint32_t freq);
void unbeep(void);
void tbeep(uint32_t freq, uint32_t ms);

#endif
