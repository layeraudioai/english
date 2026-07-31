#include "../include/json_parser.h"
#include <stdlib.h>
#include <string.h>

cJSON *read_next_json_entry(FILE *fp) {
    char *buffer = NULL;
    size_t capacity = 0;
    size_t size = 0;
    int c;

    while ((c = fgetc(fp)) != EOF) {
        if (size + 1 >= capacity) {
            capacity = capacity == 0 ? 1024 : capacity * 2;
            buffer = realloc(buffer, capacity);
        }
        buffer[size++] = (char)c;
        if (c == '\n') break;
    }

    if (size == 0) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    return json;
}
