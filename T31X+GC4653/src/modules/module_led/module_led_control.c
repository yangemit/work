#include <stdio.h>
#include <string.h>
#include <module_led_control.h>

#define MODULE_TAG              "LED_CONTROL"
static led_ctl_t led_ctx;
//输入端口
static int key_in_init(int gpio_in)//, int value)
{
	char direction_path[64] = {0};
	//char value_path[64] = {0};
	FILE *p = NULL;

	/* Robot ARM Leds.	*/
	p = fopen("/sys/class/gpio/export","w");
	if (!p)
		return -1;
	fprintf(p,"%d",gpio_in);
	fclose(p);

	sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio_in);
	//sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(direction_path, "w");
	if (!p)
		return -1;
	//fprintf(p, "out");
	fprintf(p, "in");
	fclose(p);

	/*p = fopen(value_path, "w");
	if (!p)
		return -1;
	fprintf(p, "%d", value);
	fclose(p);
	*/
	return 0;
}
//读取数据
static int read_gpio_ctl(int gpio_in)
{
	char value_path[64] = {0};
	int value = 0;
	FILE *p = NULL;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_in);

	p = fopen(value_path, "r");
	if (!p) {
		printf("ERROR(%s): fopen %s failed \n", MODULE_TAG, value_path);
		return -1;
	}
	fscanf(p,"%d", &value);
	fclose(p);
	//printf("[%s]gpio_value:%d\n",__func__,value);
	return value;
}

int module_key_ctl_read(int gpio_in)
{	int gpio_value = 0;
	gpio_value = read_gpio_ctl(gpio_in);

	return gpio_value; 
}

int module_key_in_init(int gpio_in)
{
		return key_in_init(gpio_in);
}

//输出端口
static int key_out_init(int gpio_out,int value_out)
{
	char direction_path[64] = {0};
	char value_path[64] = {0};
	FILE *p = NULL;

	/* Robot ARM Leds.	*/
	p = fopen("/sys/class/gpio/export","w");
	if (!p)
		return -1;
	fprintf(p,"%d",gpio_out);
	fclose(p);

	sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio_out);
	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_out);

	p = fopen(direction_path, "w");
	if (!p)
		return -1;
	fprintf(p, "out");
	fclose(p);

	p = fopen(value_path, "w");
	if (!p)
		return -1;
	fprintf(p, "%d", value_out);
	fclose(p);
	
	return 0;
}

static void key_ctl_write(int gpio_out, int value_out)
{
	char value_path[64] = {0};
	FILE *p = NULL;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_out);

	p = fopen(value_path, "w");
	if (!p) {
		printf("ERROR(%s): fopen %s failed \n", MODULE_TAG, value_path);
		return;
	}
	fprintf(p,"%d", value_out);
	fclose(p);
}

void module_key_ctl_write(int gpio_out, int value_out)
{

	key_ctl_write(gpio_out, value_out);
}


int module_key_out_init(int gpio_out,int value_out)
{
		return key_out_init(gpio_out,value_out);
}

//LED
static int led_init(int gpio,int value)
{
	char direction_path[64] = {0};
	char value_path[64] = {0};
	FILE *p = NULL;

	/* Robot ARM Leds.	*/
	p = fopen("/sys/class/gpio/export","w");
	if (!p)
		return -1;
	fprintf(p,"%d",gpio);
	fclose(p);

	sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio);
	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(direction_path, "w");
	if (!p)
		return -1;
	fprintf(p, "out");
	fclose(p);

	p = fopen(value_path, "w");
	if (!p)
		return -1;
	fprintf(p, "%d", value);
	fclose(p);
	
	return 0;
}

static void led_ctl(int gpio, int value)
{
	char value_path[64] = {0};
	FILE *p = NULL;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(value_path, "w");
	if (!p) {
		printf("ERROR(%s): fopen %s failed \n", MODULE_TAG, value_path);
		return;
	}
	fprintf(p,"%d", value);
	fclose(p);
}

void module_led_ctl(int gpio, int value)
{

	led_ctl(led_ctx.gpio, value);
}

int module_led_init(void *param)
{
	memset(&led_ctx, 0, sizeof(led_ctl_t));
	led_ctx.gpio = -1;
	led_ctx = *((led_ctl_t *)param);
	if (led_ctx.gpio != -1) {
		led_init(led_ctx.gpio,led_ctx.level);
	} else {
		return 1;
	}
	return 0;
}

void  module_key_deinit(int gpio)
{
	FILE *p = NULL;
	p = fopen("/sys/class/gpio/unexport","w");
	if (!p)
		return;
	fprintf(p,"%d",gpio);
	fclose(p);
	printf("INFO(%s): module_led_deinit ...ok\n", MODULE_TAG);
	return;
}

void  module_led_deinit(void)
{
	printf("INFO(%s): module_led_deinit ...ok\n", MODULE_TAG);
	return;
}
