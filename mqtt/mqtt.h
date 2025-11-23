#ifndef MQTT_H
#define MQTT_H

#include <MQTTClient.h>
#include <pthread.h>
#include <ncurses.h>

typedef struct {
    pthread_mutex_t *mutex;
    WINDOW *output_win;
} callback_context;

int mqtt_publish_message(
        MQTTClient client,
        char *topic,
        char *payload);

int connect_mqtt_client(
        MQTTClient *client,
        MQTTClient_connectOptions *conn_opts,
        char *ADDRESS,
        char *CLIENTID,
        char *USERNAME,
        char *PASSWORD,
        void *context);

int mqtt_publish_struct(
        MQTTClient client,
        char *topic,
        void *struct_data,
        int data_len);

#endif
