#include <MQTTClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>

#include "helper/helper.h"
#include "parser/parser.h"
#include "server/server.h"
#include "config/config.h"
#include "mqtt/mqtt.h"

#define MAX_LINE_LENGTH 256
#define HISTORY_FILE_NAME "history.txt"

// MQTT definitions
#define MQTT_RESPONSE_TOPIC "servers/BRAVO/response"
#define MQTT_QOS 1

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
int rc;
char response_topic_buffer[32];

// Server config struct
Global_config config;

// Mutex for writing the interface
pthread_mutex_t ncurses_mutex = PTHREAD_MUTEX_INITIALIZER;
callback_context cb_ctx;

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

        if (config->SERVER_NAME) free(config->SERVER_NAME);
        if (config->MQTT_BROKER_ADDRESS) free(config->MQTT_BROKER_ADDRESS);
        if (config->USERNAME) free(config->USERNAME);
        if (config->PASSWORD) free(config->PASSWORD);
    }
}

// Helper function to draw the input prompt in the input window
void draw_prompt(WINDOW *win) {
    wclear(win);
    mvwprintw(win, 0, 0, "%s: ", config.SERVER_NAME);
    wrefresh(win);
}

int main(){

    // Testing and debugging
    load_config_file(&config, "config.ini");

    // Initializing ncurses
    initscr(); // Init screen, inits the ncurses module
    cbreak();  // Line buffering disabled
    noecho();  // Don't echo() characters as they are typed

    int screen_rows, screen_cols;
    getmaxyx(stdscr, screen_rows, screen_cols); // Get screen size

    // Create output window (full screen - input line)
    WINDOW *out_win = newwin(screen_rows - 1, screen_cols, 0, 0);
    scrollok(out_win, TRUE); // Allow this window to scroll

    // Create input window (1 line at bottom)
    WINDOW *in_win = newwin(1, screen_cols, screen_rows - 1, 0);
    keypad(in_win, TRUE); // Enables keypad

    // Initializing MQTT
    // This context is passed to the MQTT thread's callbacks
    cb_ctx.mutex = &ncurses_mutex;
    cb_ctx.output_win = out_win;

    // For now the servers are manually written to a file, CHANGE THIS TO:
    // sv.discover -> all the servers that are listening to /servers/ALL/ping send
    // a response with their name and description.
    // This will be used to write a server struct.
    if (load_servers_from_file(&config) != 0){
        wprintw(out_win, "Failed to load servers names. Exiting.\n");
        wrefresh(out_win);
        endwin(); // Exit ncurses
        return 1;
    }

    // Connects to MQTT broker
    if ((connect_mqtt_client(&client,
                &conn_opts,
                config.MQTT_BROKER_ADDRESS,
                config.SERVER_NAME,
                config.USERNAME,
                config.PASSWORD,
                &cb_ctx)) != 0){ // Pass context struct
        endwin(); // Exit ncurses mode
        fprintf(stderr, "%s: Could not establish connection with MQTT broker.\n", config.SERVER_NAME);
        return 1;
    }

    wprintw(out_win, "%s: STARTED MQTT\n", config.SERVER_NAME);

    // Subscribing to the servers, CHANGE THIS TO:
    // for every server on the server list, subscribe to (/servers/NAME/response)
    for (int i = 0; i < config.NUM_KNOWN_SERVERS; i++) {
        snprintf(response_topic_buffer, sizeof(response_topic_buffer), "servers/%s/response", config.KNOWN_SERVERS[i]);
        wprintw(out_win, "%s - MQTT: Subscribing to %s...\n",config.SERVER_NAME, response_topic_buffer);
        if ((MQTTClient_subscribe(client, response_topic_buffer, MQTT_QOS)) != MQTTCLIENT_SUCCESS)
        {
            wprintw(out_win, "Failed to subscribe, return code %d\n", rc);
            wrefresh(out_win);
            endwin(); // Exit ncurses
            return 1; 
        }
    }

    wprintw(out_win, "%s: Loaded %d servers.\n",config.SERVER_NAME, config.NUM_KNOWN_SERVERS);
    wrefresh(out_win); // Refresh output window to show messages

    // Buffers and variables for the main loop
    char input_buffer[MAX_LINE_LENGTH];
    char buffer_copy[MAX_LINE_LENGTH]; // For strtok, it modifies the original
    char *input_buffer_tokens;
    int input_pos = 0;
    int ch;

    // Loading history variables (logging)
    FILE *history_file_pointer = fopen(HISTORY_FILE_NAME, "ab");
    if (history_file_pointer == NULL){
        endwin(); // Exit ncurses
        perror("Error opening servers file " HISTORY_FILE_NAME);
        return 1;
    }

    // Main loop starts
    draw_prompt(in_win);
    while(1){
        ch = wgetch(in_win); // Get character from input window

        switch (ch) {
            case KEY_UP:
                pthread_mutex_lock(&ncurses_mutex);
                wprintw(out_win, "Sending to server: %s\n", config.TARGET_SERVER);
                wrefresh(out_win);
                pthread_mutex_unlock(&ncurses_mutex);
                break;

            case KEY_DOWN:
                break;

            case KEY_LEFT:
                break;

            case KEY_RIGHT:
                break;

            case '\n': // Enter/RETURN
                if (input_pos == 0) {
                    continue; // Ignores when the input buffer is empty
                }

                input_buffer[input_pos] = 0; // Null-terminate

                // Write the command in the output window
                pthread_mutex_lock(&ncurses_mutex);
                wprintw(out_win, "%s: %s\n", config.SERVER_NAME, input_buffer);
                wrefresh(out_win);
                pthread_mutex_unlock(&ncurses_mutex);

                // Store in history
                fwrite(input_buffer, sizeof(char), strlen(input_buffer), history_file_pointer);
                fwrite("\n", sizeof(char), strlen("\n"), history_file_pointer);

                // Input processing: BEGIN
    
                // Process the special exit command
                if (strcmp(input_buffer, "exit") == 0) {
                    pthread_mutex_lock(&ncurses_mutex);
                    wprintw(out_win, "Exiting.\n");
                    wrefresh(out_win);
                    pthread_mutex_unlock(&ncurses_mutex);
                    goto cleanup; // Exit loop
                }

                // Creates a copy for strtok (it modifies the string)
                strcpy(buffer_copy, input_buffer);
                // Tokenizes (splits) the input buffer
                input_buffer_tokens = strtok(buffer_copy, " ");

                switch (parse_input_buffer(input_buffer_tokens)) {
                    case ERROR: 
                        pthread_mutex_lock(&ncurses_mutex);
                        wprintw(out_win, "Unknown command.\n");
                        wrefresh(out_win);
                        pthread_mutex_unlock(&ncurses_mutex);
                        break;
                    case PING_SERVER:
                        input_buffer_tokens = strtok(NULL, " "); // Next token
                        if (input_buffer_tokens == NULL){
                            pthread_mutex_lock(&ncurses_mutex);
                            wprintw(out_win, "Syntax error.\n"); 
                            wprintw(out_win, "Usage: sv.ping <SERVER NAME>\n"); 
                            wrefresh(out_win);
                            pthread_mutex_unlock(&ncurses_mutex);
                        } else{
                            // Pass mutex and output window to the function
                            ping_server(client, input_buffer_tokens, &config, &ncurses_mutex, out_win);
                        }
                        break;
                    case LIST_SERVERS:
                        input_buffer_tokens = strtok(NULL, " "); // Next token
                        // Pass mutex and output window to the function
                        list_servers(&config, &ncurses_mutex, out_win);
                        break;
                    case MQTT_ADDRESS:
                        pthread_mutex_lock(&ncurses_mutex);
                        wprintw(out_win, "MQTT BROKER ADDRESS: %s\n",config.MQTT_BROKER_ADDRESS);
                        wrefresh(out_win);
                        pthread_mutex_unlock(&ncurses_mutex);
                        break;
                    case PROGRAM_HELP:
                        pthread_mutex_lock(&ncurses_mutex);
                        wprintw(out_win, "  Server commands:\n");
                        wprintw(out_win, "      sv.list : Lists servers registered in servers.txt file\n");
                        wprintw(out_win, "      sv.ping <SERVER_NAME/ALL> : Sends ping command to specific server or to all servers. \n");
                        wprintw(out_win, "  MQTT commands:\n");
                        wprintw(out_win, "     mqtt.address : Prints to the output screen the MQTT Broker address.\n");
                        wrefresh(out_win);
                        pthread_mutex_unlock(&ncurses_mutex);
                        break;
                    case SERVER_TARGET:
                        input_buffer_tokens = strtok(NULL, " ");
                        if (input_buffer_tokens == NULL) {
                            pthread_mutex_lock(&ncurses_mutex);
                            wprintw(out_win, "Syntax error.\n"); 
                            wprintw(out_win, "Usage: sv.target <SERVER NAME>\n"); 
                            pthread_mutex_unlock(&ncurses_mutex);
                        }
                        if (server_exists(input_buffer_tokens, &config) == 1) {
                            config.TARGET_SERVER = input_buffer_tokens;
                            // TODO: IMPLEMENT UPDATE_SERVERS
                            // It should send a command to ALL requesting a updated 
                            // server struct for each server
                            // Updates all servers
                            // Updates target_server pointer
                            pthread_mutex_lock(&ncurses_mutex);
                            wprintw(out_win, "Targeting server %s...\n", input_buffer_tokens);
                            wrefresh(out_win);
                            pthread_mutex_unlock(&ncurses_mutex);
                        }
                        break;
                }

                // Reset for next command
                memset(input_buffer, 0, MAX_LINE_LENGTH);
                input_pos = 0;
                draw_prompt(in_win);
                break;

            case KEY_BACKSPACE:
            case 127: // Backspace ASCII
            case '\b': // Backspace
                if (input_pos > 0) {
                    input_pos--;
                    input_buffer[input_pos] = 0;
                    
                    // Manually delete char from input window
                    int y, x;
                    getyx(in_win, y, x);
                    mvwaddch(in_win, y, x - 1, ' ');
                    wmove(in_win, y, x - 1);
                    wrefresh(in_win);
                }
                break;

            default: // Regular character
                if (ch != ERR && input_pos < MAX_LINE_LENGTH - 1) {
                    input_buffer[input_pos++] = (char)ch;
                    waddch(in_win, (char)ch); // Echo to input window
                    wrefresh(in_win);
                }
                break;
        }
    }

cleanup:
    // Closes history file and cleans
    fclose(history_file_pointer);

    // Free memory
    cleanup_config(&config);

    // Ncurses Cleanup
    delwin(in_win);
    delwin(out_win);
    endwin(); 

    // Clean up mutex
    pthread_mutex_destroy(&ncurses_mutex); 
    return 0;
}
