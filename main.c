#include "include/english.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: english \"prompt\"\n");
        return 1;
    }

    EnglishEngine engine;
    initialize_engine(&engine);

    char *prompt = argv[1];
    
    // Check if we are already in a project directory or need to setup
    // For now, just process the prompt.
    printf("Processing prompt: %s\n", prompt);
    process_prompt(&engine, prompt);

    return 0;
}
