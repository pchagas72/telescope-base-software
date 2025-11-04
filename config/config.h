#ifndef CONFIG_H
#define CONFIG_H

typedef struct global_config{
    char **KNOWN_SERVERS;
    int NUM_KNOWN_SERVERS;

} Global_config;

int load_servers_from_file(Global_config *config);

#endif
