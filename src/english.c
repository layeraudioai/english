#include "../include/english.h"
#include "../include/json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#endif

void get_exe_dir(char *dir, size_t size) {
#ifdef _WIN32
    GetModuleFileName(NULL, dir, (DWORD)size);
    char *last_slash = strrchr(dir, '\\');
    if (last_slash) *last_slash = '\0';
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        char *last_slash = strrchr(path, '/');
        if (last_slash) *last_slash = '\0';
        strncpy(dir, path, size);
    }
#endif
}

char *find_data_file() {
    char exe_dir[1024];
    get_exe_dir(exe_dir, sizeof(exe_dir));

    char *paths[] = {
        "%s/raw-wiktextract-data.jsonl",
        "%s/wiktionary/raw-wiktextract-data.jsonl",
        "%s/deps/wiktionary/raw-wiktextract-data.jsonl"
    };

    char full_path[2048];
    for (int i = 0; i < 3; i++) {
        snprintf(full_path, sizeof(full_path), paths[i], exe_dir);
        if (access(full_path, F_OK) == 0) {
            return strdup(full_path);
        }
    }
    return NULL;
}

void initialize_engine(EnglishEngine *engine) {
    engine->data_path = find_data_file();
    if (!engine->data_path) {
        fprintf(stderr, "Error: Could not find raw-wiktextract-data.jsonl\n");
        exit(1);
    }
    engine->project_dir = ".";
    printf("English Engine initialized. Data path: %s\n", engine->data_path);
}

void process_prompt(EnglishEngine *engine, const char *prompt) {
    printf("Engine is processing: %s\n", prompt);
    // For now, let's treat the first word of the prompt as the word to look up.
    WordAssembly *wa = get_word_assembly(engine, prompt);
    if (wa) {
        printf("Definition of '%s': %s\n", prompt, wa->definition);
        free_word_assembly(wa);
    } else {
        printf("Definition not found.\n");
    }
    start_interactive_chat(engine);
}

char *lookup_word_definition(EnglishEngine *engine, const char *word) {
    FILE *fp = fopen(engine->data_path, "r");
    if (!fp) return NULL;

    cJSON *entry;
    char *definition = NULL;
    while ((entry = read_next_json_entry(fp)) != NULL) {
        cJSON *w = cJSON_GetObjectItem(entry, "word");
        if (w && strcmp(w->valuestring, word) == 0) {
            cJSON *senses = cJSON_GetObjectItem(entry, "senses");
            if (cJSON_GetArraySize(senses) > 0) {
                cJSON *sense = cJSON_GetArrayItem(senses, 0);
                cJSON *glosses = cJSON_GetObjectItem(sense, "glosses");
                if (cJSON_GetArraySize(glosses) > 0) {
                    definition = strdup(cJSON_GetArrayItem(glosses, 0)->valuestring);
                }
            }
            cJSON_Delete(entry);
            break;
        }
        cJSON_Delete(entry);
    }
    fclose(fp);
    return definition;
}

void start_interactive_chat(EnglishEngine *engine) {
    printf("Interactive chat started. Type 'exit' to quit.\n");
    char buffer[1024];
    while(1) {
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strcmp(buffer, "exit") == 0) break;
        
        // Lookup word in interactive mode
        WordAssembly *wa = get_word_assembly(engine, buffer);
        if (wa) {
            printf("Definition: %s\n", wa->definition);
            printf("Assembly: \n%s\n", wa->assembly_code);
            free_word_assembly(wa);
        } else {
            printf("Definition not found.\n");
        }
    }
}
