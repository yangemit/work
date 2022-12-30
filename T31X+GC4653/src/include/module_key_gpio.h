#ifndef __MODULE_KEY_GPIO__H
#define __MODULE_KEY_GPIO__H

int module_key_process(void);
void module_key_ctl_write(int gpio_out, int value_out);
int module_key_out_init(int gpio_out,int value_out);
int module_key_in_init(int gpio_in);
int module_key_ctl_read(int gpio_in);
#endif

