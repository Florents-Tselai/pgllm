#ifndef PGLLM_H
#define PGLLM_H


enum modelType {
    genModel,
    embedModel
};

typedef struct {
    char *name;
    char *help;

    enum modelType type;

    union {
        struct {
            text *(*generate)(void *params, char *prompt, int prompt_len);
        } generative;

        struct {
            float *(*embed)(void *params, char *prompt, int prompt_len);
        } embedding;
    } func;
} pgllm_model;

/* Every string value in the jsonb is considered a prompt  */
extern text *jsonb_transform_llm_generate(void *state, char *prompt, int prompt_len);

text *impl_repeat_3(void *params, char *prompt, int prompt_len);

text *impl_pyupper(void *params, char *prompt, int prompt_len);


static pgllm_model PGLLM_MODELS_CATALOG[] = {
        {"repeat-3", NULL, genModel, {.generative = {.generate = impl_repeat_3}}},
        {"pyupper",  NULL, genModel, {.generative = {.generate = impl_pyupper}}}
};


pgllm_model *find_model(char *name);

pgllm_model *find_model(char *name) {
    size_t i, j;  // Declare inner loop variable
    char **aliases;  // Pointer for aliases

    // Iterate through the models_catalog array
    for (i = 0; i < sizeof(PGLLM_MODELS_CATALOG) / sizeof(PGLLM_MODELS_CATALOG[0]); i++) {
        // Check if the model name matches
        if (strcmp(PGLLM_MODELS_CATALOG[i].name, name) == 0) {
            return &PGLLM_MODELS_CATALOG[i];  // Return the model if name matches
        }

//         Check if any of the aliases match
//        aliases = PGLLM_MODELS_CATALOG[i].aliases; // Assign the aliases pointer
//        if (aliases != NULL) {
//            for (j = 0; aliases[j] != NULL; j++) {
//                if (strcmp(aliases[j], name) == 0) {
//                    return &PGLLM_MODELS_CATALOG[i];  // Return the model if an alias matches
//                }
//            }
//        }
    }

    return NULL;  // Return NULL if no match is found
}

#endif
