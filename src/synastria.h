#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void synastria_enter_process(void* self, void* msg);
void synastria_leave_process(void);
void synastria_on_handle_data(void* self, void* data, int size);
void synastria_on_glue_idle(void);
void synastria_on_game_idle(void);

#ifdef __cplusplus
}
#endif
