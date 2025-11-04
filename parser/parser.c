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

    return ERROR;
}
