#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SERVER_NAME_LENGTH 64
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
