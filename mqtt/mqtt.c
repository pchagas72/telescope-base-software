#include "mqtt.h"
#include <MQTTClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/time.h>
#include "../protocol/protocol.h"
#include "../helper/helper.h"

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    callback_context *ctx = (callback_context *)context;
    pthread_mutex_lock(ctx->mutex);

    if (message->payloadlen == sizeof(Message_Struct)) {
        Message_Struct *incoming = (Message_Struct *)message->payload;

        if (incoming->type == PACKET_TYPE_PING) {
            bool integrity_ok = true;
            long long now = get_time_ms();
            long long diff = now - incoming->timestamp;
            for(int i=0; i<32; i++) {
                if (incoming->payload[i] != (char)0xAA) {
                    integrity_ok = false;
                    break;
                }
            }

            if (integrity_ok) {
                wprintw(ctx->output_win, "[%s] PACKET_RECEIVED | ID:%d | Time:%lld ms | Integrity: OK\n", 
                        topicName,incoming->packet_id, diff);
            } else {
                wprintw(ctx->output_win, "[%s] PACKET_RECEIVED | ID:%d | DATA CORRUPTED!\n",
                        topicName,
                        incoming->packet_id);
            }
        }
    } else {
        wprintw(ctx->output_win, "[%s] Text: %.*s\n", topicName, message->payloadlen, (char*)message->payload);
    }

    wrefresh(ctx->output_win);

    // Unlock mutex
    pthread_mutex_unlock(ctx->mutex);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    // Get context from the ncurses window
    callback_context *ctx = (callback_context *)context;

    // Lock mutex
    pthread_mutex_lock(ctx->mutex);

    // Writes and updates window
    wprintw(ctx->output_win, "\nConnection lost\n");
    wprintw(ctx->output_win, "    cause: %s\n", cause);
    wrefresh(ctx->output_win);

    // Unlocks mutex
    pthread_mutex_unlock(ctx->mutex); // <-- Unlock
}

int connect_mqtt_client(
        MQTTClient *client,
        MQTTClient_connectOptions *conn_opts,
        char *ADDRESS,
        char *CLIENTID,
        char *USERNAME,
        char *PASSWORD,
        void *context
        ){
    int rc;

    // Creating client
    if ((rc = MQTTClient_create(client, ADDRESS, CLIENTID,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS){
        // Critical errors are printed in STDERR
        fprintf(stderr, "PAHOMQTT: Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        return 1;
    }

    // MQTTClient_setCallbacks(client, contex, lost connection, message arrived, message delivered)
    if ((rc = MQTTClient_setCallbacks(*client, context, connlost, msgarrvd, NULL)) != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "PAHOMQTT: Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        MQTTClient_destroy(client); 
        return 1;
    }

    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    ssl_opts.enableServerCertAuth = 1;
    ssl_opts.trustStore = "/etc/ssl/certs/ca-certificates.crt";

    if (strncmp(ADDRESS, "ssl://", 6) == 0 || strncmp(ADDRESS, "mqtts://", 8) == 0){
        conn_opts->ssl = &ssl_opts;
    }

    conn_opts->username = USERNAME;
    conn_opts->password = PASSWORD;

    // Needs to be stated, or else ERROR PAHOmqtt -14 might happen
    conn_opts->MQTTVersion = MQTTVERSION_3_1_1;

    if ((rc = MQTTClient_connect(*client, conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "PAHOMQTT: Failed to connect, return code %d\n",rc);
        if (conn_opts->ssl != NULL) {
            fprintf(stderr, "Note: SSL connection failed. Check certificates and protocol prefix (ssl://).\n");
        }
        rc = EXIT_FAILURE;
        return 1;
    }
 
    return 0;
}

int mqtt_publish_message(MQTTClient client, char *topic, char *payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = 1; // Using QOS 1
    pubmsg.retained = 0;

    if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to publish message, return code %d\n", rc);
        return rc;
    }

    return MQTTCLIENT_SUCCESS;
}

int mqtt_publish_struct(MQTTClient client, char *topic, void *struct_data, int data_len) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    pubmsg.payload = struct_data;
    pubmsg.payloadlen = data_len;
    pubmsg.qos = 1;
    pubmsg.retained = 0;

    if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
        // Log error
        return rc;
    }
    return MQTTCLIENT_SUCCESS;
}
