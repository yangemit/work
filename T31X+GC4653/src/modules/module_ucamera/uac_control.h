#ifndef __UAC_CONTROL__H
#define __UAC_CONTROL__H

int uac_control_init(void *param);
void uac_control_deinit();
void set_audio_switch(int on_off);
#endif
