#ifndef __MODULE_LED_CONTROL__H
#define __MODULE_LED_CONTROL__H
#include <stdint.h>
#include <global_config.h>

int module_key_ctl_read(int gpio_in);
void module_key_ctl_write(int gpio_out, int value_out);
int module_led_init(void *param);
void module_led_deinit(void);
void module_led_ctl(int gpio, int value);
int module_key_in_init(int gpio_in);
int module_key_out_init(int gpio_out,int value_out);
void  module_key_deinit(int gpio);

#endif

