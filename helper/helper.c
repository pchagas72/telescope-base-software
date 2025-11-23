#include "helper.h"
#include <string.h>
#include <sys/time.h>

long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

int server_exists(char *server_name, Global_config *config){
    int server_exists = 0;
    for (int i = 0; i < config->NUM_KNOWN_SERVERS; i++) {
        if (strcmp(server_name, config->KNOWN_SERVERS[i]) == 0) {
            server_exists = 1; 
        }
        if (strcmp(server_name, "ALL") == 0 && config->NUM_KNOWN_SERVERS > 0) {
            server_exists = 2;
        }
    }
    return server_exists;
}
