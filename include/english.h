#ifndef ENGLISH_H
#define ENGLISH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Represents an assembly-level implementation for a word
typedef struct {
    char *word;
    char *definition;
    char *assembly_code;
    char *structure;
    char *ref_explanation;
    char *self_contained_explanation;
} WordAssembly;

// Core engine structure
typedef struct {
    // Path to the wiktionary data
    char *data_path;
    // Current project directory
    char *project_dir;
} EnglishEngine;

// Function prototypes
void initialize_engine(EnglishEngine *engine);
void process_prompt(EnglishEngine *engine, const char *prompt);
void start_interactive_chat(EnglishEngine *engine);
char *lookup_word_definition(EnglishEngine *engine, const char *word);
WordAssembly *get_word_assembly(EnglishEngine *engine, const char *word);
void free_word_assembly(WordAssembly *wa);

#endif // ENGLISH_H
