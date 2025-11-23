#include "server.h"
#include <bits/types/struct_timeval.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "../mqtt/mqtt.h"
#include <pthread.h>
#include <ncurses.h>
#include "../helper/helper.h"
#include "../protocol/protocol.h"

int ping_server(
        MQTTClient client,
        char *server_name,
        Global_config *config,
        pthread_mutex_t *mutex,
        WINDOW *output_win){

    int whom_to_ping = server_exists(server_name, config);

    Message_Struct ping_packet;
    ping_packet.type = PACKET_TYPE_PING;
    ping_packet.packet_id = rand(); // Change to counter later

    struct timeval tv; 
    gettimeofday(&tv, NULL);
    ping_packet.timestamp = (long long)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

    memset(ping_packet.payload, 0xAA, sizeof(ping_packet.payload));

    char topic_buffer[128];

    if(whom_to_ping == 1){
        snprintf(topic_buffer, 128, "servers/%s/command", server_name);
    } else if(whom_to_ping == 2){
        snprintf(topic_buffer, 128, "servers/ALL/command");
    } else {
        pthread_mutex_lock(mutex);
        wprintw(output_win, "Server %s is not registered.\n", server_name);
        wrefresh(output_win);
        pthread_mutex_unlock(mutex);
        return 1;
    }

    pthread_mutex_lock(mutex);
    wprintw(output_win, "Sending Ping ID: %d to %s...\n", ping_packet.packet_id, server_name);
    wrefresh(output_win);
    pthread_mutex_unlock(mutex);

    return mqtt_publish_struct(client, topic_buffer, &ping_packet, sizeof(Message_Struct));
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
