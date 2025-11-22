#include "mqtt.h"
#include <MQTTClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    // Get ncurses window context
    callback_context *ctx = (callback_context *)context;

    // Mutex locks
    pthread_mutex_lock(ctx->mutex);

    // Writes and updates window
    wprintw(ctx->output_win, "[%s] %.*s\n",topicName, message->payloadlen, (char*)message->payload);
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

    // You can optionally wait for delivery confirmation, 
    // but for commands, "fire and forget" is often fine.
    // printf("Waiting for publication of %s\n", payload);
    // rc = MQTTClient_waitForCompletion(client, token, 10000L);
    // printf("Message with token %d delivered\n", token);
    
    return MQTTCLIENT_SUCCESS;
}
