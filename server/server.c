#include "server.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../mqtt/mqtt.h"
#include <pthread.h>
#include <ncurses.h>

int ping_server(
        MQTTClient client,
        char *server_name,
        Global_config *config,
        pthread_mutex_t *mutex,
        WINDOW *output_win){

    bool server_exists = false;
    bool ping_all = false;
    for (int i = 0; i < config->NUM_KNOWN_SERVERS; i++) {
        if (strcmp(server_name, config->KNOWN_SERVERS[i]) == 0) {
            server_exists = true; 
        }
        if (strcmp(server_name, "ALL") == 0) {
            ping_all = true; 
        }
    }

    char topic_buffer[128];
    char *payload = "sv.ping";


    if(ping_all){
        // Locking interface mutex
        pthread_mutex_lock(mutex);

        // Writes and updates the interface
        wprintw(output_win, "Pinging ALL servers...\n");
        wrefresh(output_win);
                            
        // Unlock interface mutex
        pthread_mutex_unlock(mutex);

        // Sends message
        snprintf(topic_buffer, 128, "servers/ALL/command");
        mqtt_publish_message(client, topic_buffer, payload);
        return 0;
    }

    if(server_exists){
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
