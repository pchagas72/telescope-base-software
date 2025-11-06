#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SERVER_NAME_LENGTH 64
#define MAX_INPUT_BUFFER_LINE 128
#define SERVER_FILE_NAME "servers.txt"

int load_servers_from_file(Global_config *config) {
    FILE *file = fopen(SERVER_FILE_NAME, "r");
    if (file == NULL) {
        perror("Error opening servers file " SERVER_FILE_NAME);
        return 1;
    }

    int num_servers = 0;
    char line[MAX_SERVER_NAME_LENGTH];

    // First pass: Count the number of servers
    while (fgets(line, sizeof(line), file) != NULL) {
        // Avoid counting empty lines
        if (strcspn(line, " \t\r\n") != 0) {
            num_servers++;
        }
    }

    // Allocate memory for the array of server name pointers
    config->KNOWN_SERVERS = (char **)malloc(num_servers * sizeof(char *));
    if (config->KNOWN_SERVERS == NULL) {
        perror("Failed to allocate memory for server list");
        fclose(file);
        return 1;
    }
    config->NUM_KNOWN_SERVERS = num_servers;

    // Second pass: Read server names and store them
    rewind(file);
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Trim trailing whitespace (good practice)
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\r')) {
            line[--len] = 0;
        }

        if (len > 0) {
            // Allocate memory for the server name string and copy it
            config->KNOWN_SERVERS[i] = (char *)malloc(len + 1);
            if (config->KNOWN_SERVERS[i] == NULL) {
                perror("Failed to allocate memory for server name");
                // Cleanup already allocated memory
                for (int j = 0; j < i; j++) {
                    free(config->KNOWN_SERVERS[j]);
                }
                free(config->KNOWN_SERVERS);
                config->KNOWN_SERVERS = NULL;
                config->NUM_KNOWN_SERVERS = 0;
                fclose(file);
                return 1;
            }
            strcpy(config->KNOWN_SERVERS[i], line);
            i++;
        }
    }

    fclose(file);

    return 0;
}

int load_config_file(Global_config *config, char *file_path){
    FILE *file = fopen(file_path, "r");
    if (file==NULL){
        perror("Could not read configuration file.\n");
        return 1;
    }

    Parser p;
    config->NUM_KNOWN_SERVERS = 0;
    char input_buffer[MAX_INPUT_BUFFER_LINE];
    p.state = STATE_UNKNOWN;

    while (fgets(input_buffer, sizeof(input_buffer), file) != NULL) {

        // Removes \n's
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // Ignores whitespaces
        char *line = input_buffer;
        while (*line == ' ' || *line == '\t') {
            line++;
        }

        // Skip empty lines or comments
        if (line[0] == '\0' || line[0] == '#'){
            continue;
        }

        if (strncmp(input_buffer, "SERVERS:", 8) == 0){
            p.state = STATE_SERVER;
            continue;
        }

        if (strncmp(input_buffer, "MQTT:", 5) == 0){
            p.state = STATE_MQTT;
            continue;
        }

        switch (p.state) {
            case STATE_SERVER:
                if (strncmp(line, "SERVER_NAME=", 12)==0) {
                    char *value = line + 12; 

                    size_t len = strlen(value);
                    while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                        value[--len] = 0;
                    }

                    if (config->SERVER_NAME) free(config->SERVER_NAME);
                    config->SERVER_NAME = (char*)malloc(len+1);

                    if (config->SERVER_NAME == NULL) {
                        perror("Failed to allocate memory for SERVER_NAME");
                        fclose(file);
                        return 1;
                    }
                    strcpy(config->SERVER_NAME, value);
                }
                break;
            case STATE_MQTT:
                printf("Found MQTT: %s\n", input_buffer);
                break;
            case STATE_UNKNOWN:
                break;
        }
    
    }

    config->MQTT_BROKER_ADDRESS="192.168.1.3:1883"; 
    config->MQTT_QOS= 1;

    return 0;
}
