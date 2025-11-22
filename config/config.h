#ifndef CONFIG_H
#define CONFIG_H


typedef struct global_config{
    char **KNOWN_SERVERS;
    int NUM_KNOWN_SERVERS;
    char *SERVER_NAME;
    char *MQTT_BROKER_ADDRESS;
    int MQTT_QOS;
    char *PASSWORD;
    char *USERNAME;
} Global_config;

int load_servers_from_file(Global_config *config);
int load_config_file(Global_config *config, char *file_path);

#endif
