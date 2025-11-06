#ifndef PARSER_H
#define PARSER_H

enum instructions{
    PING_SERVER,
    LIST_SERVERS,
    MQTT_ADDRESS,
    PROGRAM_HELP,
    ERROR,
};

enum instructions parse_input_buffer(char *input_buffer);

#endif
