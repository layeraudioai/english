#ifndef ASSEMBLY_H
#define ASSEMBLY_H

struct EnglishEngine; // Forward declaration

// Represents the full assembly-level details for a word
typedef struct {
    char *word;
    char *definition;
    char *assembly_code;
    char *structure;
    char *ref_explanation;
    char *self_contained_explanation;
} WordAssembly;

WordAssembly *get_word_assembly(struct EnglishEngine *engine, const char *word);
void free_word_assembly(WordAssembly *wa);

#endif // ASSEMBLY_H