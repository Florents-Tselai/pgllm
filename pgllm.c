#include "postgres.h"
#include "utils/jsonb.h"
#include "utils/jsonfuncs.h"

#include "utils/builtins.h"

#include "Python.h"

PG_MODULE_MAGIC;

void
_PG_init(void) {
    Py_Initialize();
}

/* Tear-down */
void _PG_fini(void) {
    Py_Finalize();
}

PG_FUNCTION_INFO_V1(pyupper);

Datum
pyupper(PG_FUNCTION_ARGS) {
//    text *t = PG_GETARG_TEXT_PP(0);
    PyObject *py_str = NULL, *py_upper_str = NULL;
    char *str = text_to_cstring(PG_GETARG_TEXT_P(0));
    char *upper_str = NULL;
    int32 len;


    /*
     * Create a Python string from the C string.
     */
    py_str = PyUnicode_FromString(str);
    if (!py_str) {
        PyErr_Print();
        elog(ERROR, "Failed to convert C string to Python string");
    }

    /*
     * Call the upper() method on the Python string.
     */
    py_upper_str = PyObject_CallMethod(py_str, "upper", NULL);
    if (!py_upper_str) {
        PyErr_Print();
        elog(ERROR, "Failed to call Python upper() method");
    }

    /*
     * Convert the resulting Python string back to a C string.
     */
    upper_str = PyUnicode_AsUTF8(py_upper_str);
    if (!upper_str) {
        PyErr_Print();
        elog(ERROR, "Failed to convert Python string to C string");
    }

    /*
     * Create a new PostgreSQL text object from the uppercase C string.
     */
    len = strlen(upper_str);
    text *new_t = (text *) palloc(len + VARHDRSZ);
    SET_VARSIZE(new_t, len + VARHDRSZ);
    memcpy(VARDATA(new_t), upper_str, len);

    /*
     * Clean up Python objects and finalize the interpreter.
     */
    Py_XDECREF(py_upper_str);
    Py_XDECREF(py_str);

    pfree(str);

    PG_RETURN_TEXT_P(new_t);
}

typedef struct {
    const char *model;
    const Jsonb *params;
} LLMState;


/* Function to get an integer from a Jsonb object using a key */
int jsonb_get_int(Jsonb *jb, char *key, int default_) {
    return 3;
}


static text *jsonb_transform_llm_generate(void *state, char *elem, int elem_len) {
    LLMState *my_state = (LLMState *) state; // Cast to the correct type
    const char *model = my_state->model;
    Jsonb *params = my_state->params;
    int n;//= jsonb_get_int(params, "n", 3);

    text *result = NULL;

    if (strcmp(model, "repeat-1") == 0) {
        n = 1;
        int upper_len = elem_len * n; // Calculate the length of the resulting string
        result = palloc(VARHDRSZ + upper_len);
        SET_VARSIZE(result, VARHDRSZ + upper_len);

        // Fill the result with the repeated uppercase string
        char *result_ptr = VARDATA(result);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < elem_len; j++) {
                result_ptr[i * elem_len + j] = toupper(elem[j]);
            }
        }
    } else if (strcmp(model, "repeat-3") == 0) {
        n = 3;
        int upper_len = elem_len * n; // Calculate the length of the resulting string
        result = palloc(VARHDRSZ + upper_len);
        SET_VARSIZE(result, VARHDRSZ + upper_len);

        // Fill the result with the repeated uppercase string
        char *result_ptr = VARDATA(result);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < elem_len; j++) {
                result_ptr[i * elem_len + j] = toupper(elem[j]);
            }
        }
    } else {
        elog(ERROR, "pglmm: model %s not supported\n", model);
    }

    return result;
}

PG_FUNCTION_INFO_V1(jsonb_llm_generate);

Datum
jsonb_llm_generate(PG_FUNCTION_ARGS) {
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    char *model = text_to_cstring(PG_GETARG_TEXT_P(1));
    Jsonb *params = PG_GETARG_JSONB_P(2);
    Jsonb *res = NULL;

    LLMState *llmCtxt = palloc(sizeof(LLMState));
    llmCtxt->model = model;
    llmCtxt->params = params;

    res = transform_jsonb_string_values(
            jb,
            llmCtxt,
            jsonb_transform_llm_generate
    );

    pfree(llmCtxt);

    PG_RETURN_JSONB_P(res);
}

PG_FUNCTION_INFO_V1(jsonb_llm_embed);

Datum jsonb_llm_embed(PG_FUNCTION_ARGS) {

    PG_RETURN_JSONB_P(PG_GETARG_JSONB_P_COPY(0));
}