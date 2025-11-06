#ifndef CONFIG_H
#define CONFIG_H

enum ParserState{
    STATE_SERVER,
    STATE_MQTT,
    STATE_UNKNOWN
};

typedef struct global_config{
    char **KNOWN_SERVERS;
    int NUM_KNOWN_SERVERS;
    char *SERVER_NAME;
    char *MQTT_BROKER_ADDRESS;
    int MQTT_QOS;
} Global_config;

typedef struct parser{
    enum ParserState state;
    int line_number;
} Parser;

int load_servers_from_file(Global_config *config);
int load_config_file(Global_config *config, char *file_path);

#endif
