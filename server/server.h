#ifndef SERVER_H
#define SERVER_H

#include "../config/config.h"
#include <MQTTClient.h>
#include <pthread.h>
#include <ncurses.h>

typedef struct server {
    int status;
} Server;

int ping_server(
        MQTTClient client,
        char *input_buffer,
        Global_config *config,
        pthread_mutex_t *mutex,
        WINDOW *output_win);

void list_servers(Global_config *config,
        pthread_mutex_t *mutex,
        WINDOW *output_win);

#endif
