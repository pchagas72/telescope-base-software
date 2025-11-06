#include "parser.h"
#include <string.h>

enum instructions parse_input_buffer(char *input_buffer){

    // Parsing server ping commands
    if (strstr(input_buffer, "sv.ping")) {
        return PING_SERVER;
    }

   if (strstr(input_buffer, "sv.list")) {
        return LIST_SERVERS;
    }

   if (strstr(input_buffer, "mqtt.address")) {
        return MQTT_ADDRESS;
    }

   if (strstr(input_buffer, "help")) {
        return PROGRAM_HELP;
    }

    return ERROR;
}
