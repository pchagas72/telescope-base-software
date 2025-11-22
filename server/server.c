#include "server.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../mqtt/mqtt.h"
#include <pthread.h>
#include <ncurses.h>
#include "../helper/helper.h"

int ping_server(
        MQTTClient client,
        char *server_name,
        Global_config *config,
        pthread_mutex_t *mutex,
        WINDOW *output_win){

    int whom_to_ping = server_exists(server_name, config);

    char topic_buffer[128];
    char payload[64];
    snprintf(payload, 64, "sv.ping");

    if(whom_to_ping == 2){
        pthread_mutex_lock(mutex);
        wprintw(output_win, "Pinging ALL servers...\n");
        wrefresh(output_win);
        pthread_mutex_unlock(mutex);

        snprintf(topic_buffer, 128, "servers/ALL/command");
        mqtt_publish_message(client, topic_buffer, payload);
        return 0;
    }

    if(whom_to_ping == 1){
        pthread_mutex_lock(mutex);
        wprintw(output_win, "Pinging %s...\n", server_name);
        wrefresh(output_win);
        pthread_mutex_unlock(mutex);
        snprintf(topic_buffer, 128, "servers/%s/command", server_name);
        mqtt_publish_message(client, topic_buffer, payload);
        return 0;
    }

    pthread_mutex_lock(mutex);
    wprintw(output_win, "Server %s is not registered.\n", server_name);
    wrefresh(output_win);
    pthread_mutex_unlock(mutex);
    return 1;
}

void list_servers(Global_config *config, pthread_mutex_t *mutex, WINDOW *output_win){
    pthread_mutex_lock(mutex);
    wprintw(output_win, "Servers registered:\n");
    for (int i = 0; i < config->NUM_KNOWN_SERVERS; i++) {
        wprintw(output_win, "Server %d: %s\n", i+1, config->KNOWN_SERVERS[i]);
    }
    wrefresh(output_win);
    pthread_mutex_unlock(mutex);
}
