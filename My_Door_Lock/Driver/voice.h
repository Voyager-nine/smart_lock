#ifndef DRIVER_VOICE_H_
#define DRIVER_VOICE_H_

#include "mydefine.h"

#define VOICE_TRACK_COUNT 21
#define VOICE_TRACK_KEY 1
#define VOICE_TRACK_LOCKED 2
#define VOICE_TRACK_PASSWORD_CORRECT 3
#define VOICE_TRACK_PASSWORD_WRONG 4
#define VOICE_TRACK_PASSWORD_LOCKED 5

void voice_init(void);
void voice_play(uint8_t num);

#endif /* DRIVER_VOICE_H_ */
