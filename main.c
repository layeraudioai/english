#include "include/english.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: english \"<prompt>\" | --preprocess\n");
        return 1;
    }

    EnglishEngine engine;
    initialize_engine(&engine);

    if (strcmp(argv[1], "--preprocess") == 0) {
        pre_process_wiktionary(&engine);
    } else {
        char *prompt = argv[1];
        
        // Check if we are already in a project directory or need to setup
        // For now, just process the prompt.
        printf("Processing prompt: %s\n", prompt);
        process_prompt(&engine, prompt);
    }

    return 0;
}
