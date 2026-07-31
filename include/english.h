#ifndef ENGLISH_H
#define ENGLISH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembly.h"

// Forward declare cJSON to avoid including the full header here
struct cJSON;

// Represents an assembly-level implementation for a word
typedef struct {
    char *word;
    char *assembly_code;
    char *explanation;
} WordImplementation;

// Core engine structure
typedef struct EnglishEngine {
    // Path to the wiktionary data
    char *data_path;
    // Current project directory
    char *project_dir;
} EnglishEngine;

// Function prototypes
void initialize_engine(EnglishEngine *engine);
void process_prompt(EnglishEngine *engine, const char *prompt);
void start_interactive_chat(EnglishEngine *engine); // This will be called from main now
char *lookup_word_definition(EnglishEngine *engine, const char *word);
char *slow_lookup_definition_from_jsonl(EnglishEngine *engine, const char *word); 
void cache_word_files(const char *word, struct cJSON *entry);
void pre_process_wiktionary(EnglishEngine *engine);

#endif // ENGLISH_H