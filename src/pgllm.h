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

    void (*validate_params)(Jsonb *params);

    Jsonb *params;
} LlmModelCtxt;

/* Every string value in the jsonb is considered a prompt  */
extern text *jsonb_transform_llm_generate(void *state, char *prompt, int prompt_len);

text *repeat_n_generate_internal(void *params, char *prompt, int prompt_len);

#ifdef WITH_LLAMAFILE
#include <curl/curl.h>
text *llamafile_generate_internal(void *params, char *prompt, int prompt_len);

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;


size_t write_callback(void *ptr, size_t size, size_t nmemb, MemoryStruct *result);


#endif

text *impl_pyupper(void *params, char *prompt, int prompt_len);


static LlmModelCtxt PGLLM_MODELS_CATALOG[] = {
        {"repeat-3", NULL, genModel, {.generative = {.generate = repeat_n_generate_internal}}, NULL, NULL}
#ifdef WITH_LLAMAFILE
        ,{"llamafile", NULL, genModel, {.generative = {.generate = llamafile_generate_internal}}, NULL, NULL}
#endif

};


LlmModelCtxt *search_models_catalog(char *name);

inline LlmModelCtxt *search_models_catalog(char *name) {
    size_t i, j; // Declare inner loop variable
    char **aliases; // Pointer for aliases

    // Iterate through the models_catalog array
    for (i = 0; i < sizeof(PGLLM_MODELS_CATALOG) / sizeof(PGLLM_MODELS_CATALOG[0]); i++) {
        // Check if the model name matches
        if (strcmp(PGLLM_MODELS_CATALOG[i].name, name) == 0) {
            return &PGLLM_MODELS_CATALOG[i]; // Return the model if name matches
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

    return NULL; // Return NULL if no match is found
}

#endif
