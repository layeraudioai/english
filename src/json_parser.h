#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "../deps/cjson/cJSON.h"
#include <stdio.h>

// Reads the next line from the file and parses it as JSON
cJSON *read_next_json_entry(FILE *fp);

#endif // JSON_PARSER_H
