#ifndef __MODULE_CONFIG_H__
#define __MODULE_CONFIG_H__

#define MAX_CONFIG_FILE_SIZE              4096
#define MAX_KEY_LENGTH                    16
#define MAX_VALUE_LENGTH                  96

#define CONFIG_OPS_SUCCESS                0
#define CONFIG_OPS_FAIL                   -1
#define CONFIG_RECORD_NOT_EXSIT           -2

int module_config_init(char *cfg_file_path);
int module_config_deinit(void);

/* add new record */
int module_config_add_string(const char *key, char *value);
int module_config_add_int(const char *key, int value);
int module_config_add_float(const char *key, float value);

/**
 * update record
 * if record not exist and create == 1,
 * it will add the new record to config file.
 **/
int module_config_update_string(const char *key, char *value, int create);
int module_config_update_int(const char *key, int value, int create);
int module_config_update_float(const char *key, float value, int create);

/* get existed record */
int module_config_get_string(const char *key, char *value);
int module_config_get_int(const char *key, int *value);
int module_config_get_float(const char *key, float *value);

/* delete record */
int module_config_delete(const char *key);


#endif /* __MODULE_CONFIG_H__ */
