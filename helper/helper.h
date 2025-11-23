#ifndef HELPER_H
#define HELPER_H

#include "../config/config.h"

int server_exists(char *server_name, Global_config *config);
long long get_time_ms(void);

#endif
