#include "../include/english.h"
#include "../include/json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <limits.h>
#endif

void get_exe_dir(char *dir, size_t size) {
#ifdef _WIN32
    GetModuleFileName(NULL, dir, (DWORD)size);
    char *last_slash = strrchr(dir, '\\');
    if (last_slash) *(last_slash + 1) = '\0';
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        char *last_slash = strrchr(path, '/');
        if (last_slash) *(last_slash + 1) = '\0';
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
}

char *lookup_word_definition(EnglishEngine *engine, const char *word) {
    char exe_dir[1024];
    get_exe_dir(exe_dir, sizeof(exe_dir));
    char word_json_path[2048];
    snprintf(word_json_path, sizeof(word_json_path), "%swords/%s/%s.json", exe_dir, word, word);

    FILE *fp = fopen(word_json_path, "rb");
    if (!fp) {
        // Fallback to slow search for pre-processing or if file not found
        // In a future state, this might just return NULL.
        // For now, it allows pre-processing to still work.
        fp = fopen(engine->data_path, "r");
        if (!fp) return NULL;

        cJSON *entry;
        char *definition = NULL;
        while ((entry = read_next_json_entry(fp)) != NULL) {
            cJSON *w = cJSON_GetObjectItem(entry, "word");
            if (w && strcmp(w->valuestring, word) == 0) {
                // Found it, now extract definition (same logic as below)
                cJSON *senses = cJSON_GetObjectItem(entry, "senses");
                if (cJSON_GetArraySize(senses) > 0) {
                    cJSON *sense = cJSON_GetArrayItem(senses, 0);
                    cJSON *glosses = cJSON_GetObjectItem(sense, "glosses");
                    if (cJSON_GetArraySize(glosses) > 0) {
                        definition = strdup(cJSON_GetArrayItem(glosses, 0)->valuestring);
                    }
                }
                cJSON_Delete(entry);
                fclose(fp);
                return definition;
            }
            cJSON_Delete(entry);
        }
        fclose(fp);
        return NULL; // Not found in slow search either
    }

    // Efficient lookup from pre-processed file
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, fp);
    fclose(fp);
    buffer[length] = '\0';

    cJSON *entry = cJSON_Parse(buffer);
    free(buffer);

    if (!entry) return NULL;

    char *definition = NULL;
    cJSON *senses = cJSON_GetObjectItem(entry, "senses");
    if (cJSON_GetArraySize(senses) > 0) {
        cJSON *sense = cJSON_GetArrayItem(senses, 0);
        cJSON *glosses = cJSON_GetObjectItem(sense, "glosses");
        if (cJSON_GetArraySize(glosses) > 0) {
            definition = strdup(cJSON_GetArrayItem(glosses, 0)->valuestring);
        }
    }
    cJSON_Delete(entry);

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

void create_dir_if_not_exists(const char *path) {
#ifdef _WIN32
    CreateDirectory(path, NULL);
#else
    mkdir(path, 0755);
#endif
}

void write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    } else {
        fprintf(stderr, "Failed to open file for writing: %s\n", path);
    }
}

void pre_process_wiktionary(EnglishEngine *engine) {
    FILE *fp = fopen(engine->data_path, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open wiktionary file: %s\n", engine->data_path);
        return;
    }

    char exe_dir[1024];
    get_exe_dir(exe_dir, sizeof(exe_dir));
    char words_dir[1024];
    snprintf(words_dir, sizeof(words_dir), "%s/words", exe_dir);
    create_dir_if_not_exists(words_dir);

    printf("Starting wiktionary processing. This may take a while...\n");

    cJSON *entry;
    int count = 0;
    while ((entry = read_next_json_entry(fp)) != NULL) {
        cJSON *word_json = cJSON_GetObjectItem(entry, "word");
        if (!word_json || !word_json->valuestring) {
            cJSON_Delete(entry);
            continue;
        }
        char *word = word_json->valuestring;

        WordAssembly *wa = get_word_assembly(engine, word);
        if (!wa) {
            cJSON_Delete(entry);
            continue;
        }

        char word_dir[2048];
        snprintf(word_dir, sizeof(word_dir), "%s/%s", words_dir, word);
        create_dir_if_not_exists(word_dir);

        char file_path[4096];

        // Write JSON
        char *json_string = cJSON_Print(entry);
        snprintf(file_path, sizeof(file_path), "%s/%s.json", word_dir, word);
        write_file(file_path, json_string);
        free(json_string);

        // Write ASM
        snprintf(file_path, sizeof(file_path), "%s/%s.asm", word_dir, word);
        write_file(file_path, wa->assembly_code);

        // Write C file
        char c_content[2048];
        snprintf(c_content, sizeof(c_content), "#include \"%s.h\"\n\n// Implementation for %s\n", word, word);
        snprintf(file_path, sizeof(file_path), "%s/%s.c", word_dir, word);
        write_file(file_path, c_content);

        // Write H file
        char h_content[2048];
        snprintf(h_content, sizeof(h_content), "#ifndef %s_H\n#define %s_H\n\n// Definition for %s\n\n#endif // %s_H\n", word, word, word, word);
        snprintf(file_path, sizeof(file_path), "%s/%s.h", word_dir, word);
        write_file(file_path, h_content);

        free_word_assembly(wa);
        cJSON_Delete(entry);
        count++;
        if (count % 1000 == 0) printf("Processed %d words...\n", count);
    }

    printf("Finished processing. Processed %d words.\n", count);
    fclose(fp);
}
