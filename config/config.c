#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BASE_NAME_LENGTH 64
#define MAX_INPUT_BUFFER_LINE 128
#define SERVER_FILE_NAME "servers.txt"

int load_servers_from_file(Global_config *config) {
    FILE *file = fopen(SERVER_FILE_NAME, "r");
    if (file == NULL) {
        perror("Error opening servers file " SERVER_FILE_NAME);
        return 1;
    }

    int num_servers = 0;
    char line[MAX_BASE_NAME_LENGTH];

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

    config->NUM_KNOWN_SERVERS = 0;
    char input_buffer[MAX_INPUT_BUFFER_LINE];

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

        if (strncmp(line, "BASE_NAME=", 10)==0) {
            char *value = line + 10; 

            size_t len = strlen(value);
            while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                value[--len] = 0;
            }

            if (config->BASE_NAME) free(config->BASE_NAME);
            config->BASE_NAME = (char*)malloc(len+1);

            if (config->BASE_NAME == NULL) {
                perror("Failed to allocate memory for BASE_NAME");
                fclose(file);
                return 1;
            }
            strcpy(config->BASE_NAME, value);
            continue;
        }
        if (strncmp(line, "BROKER_ADDRESS=",  15)==0){
            char *value = line + 15;

            size_t len = strlen(value);
            while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                value[--len] = 0;
            }

            if (config->MQTT_BROKER_ADDRESS) free(config->MQTT_BROKER_ADDRESS);
            config->MQTT_BROKER_ADDRESS = (char*)malloc(len+1);

            if (config->MQTT_BROKER_ADDRESS == NULL) {
                perror("Failed to allocate memory for MQTT_BROKER_ADDRESS");
                fclose(file);
                return 1;
            }
            strcpy(config->MQTT_BROKER_ADDRESS, value);
            continue;

        }
        if (strncmp(line, "QOS=",  4)==0){
            char *value = line + 4;

            size_t len = strlen(value);
            while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                value[--len] = 0;
            }

            config->MQTT_QOS = atoi(value);
            continue;
        }
        if (strncmp(line, "USERNAME=", 9) == 0){
            char *value = line + 9;

            // Jumps whitelines
            size_t len = strlen(value);
            while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                value[--len] = 0;
            }

            if (config->USERNAME) free(config->USERNAME);
            config->USERNAME = (char*)malloc(len+1);

            if (config->USERNAME == NULL) {
                perror("Failed to allocate memory for USERNAME");
                fclose(file);
                return 1;
            }
            strcpy(config->USERNAME, value);
            continue;
        }
        if (strncmp(line, "PASSWORD=", 9) == 0){
            char *value = line + 9;

            // Jumps whitelines
            size_t len = strlen(value);
            while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r')) {
                value[--len] = 0;
            }

            if (config->PASSWORD) free(config->PASSWORD);
            config->PASSWORD = (char*)malloc(len+1);

            if (config->PASSWORD == NULL) {
                perror("Failed to allocate memory for PASSWORD");
                fclose(file);
                return 1;
            }
            strcpy(config->PASSWORD, value);
            continue;
       }
    }

    return 0;
}

// Frees memory allocated for the config struct
void cleanup_config(Global_config *config){
    if (config->KNOWN_SERVERS != NULL) {
        for (int i = 0; i<config->NUM_KNOWN_SERVERS; i++) {
            if (config->KNOWN_SERVERS[i] != NULL) {
                free(config->KNOWN_SERVERS[i]);
                config->KNOWN_SERVERS[i] = NULL;
            } 
        } 
        free(config->KNOWN_SERVERS);

        if (config->BASE_NAME) free(config->BASE_NAME);
        if (config->MQTT_BROKER_ADDRESS) free(config->MQTT_BROKER_ADDRESS);
        if (config->USERNAME) free(config->USERNAME);
        if (config->PASSWORD) free(config->PASSWORD);
    }
}
