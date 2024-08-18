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

void
_PG_fini(void) {
    Py_Finalize();
}


PG_FUNCTION_INFO_V1(jsonb_llm_hello);

/* by value */

PG_FUNCTION_INFO_V1(add_one);

Datum
add_one(PG_FUNCTION_ARGS) {
    int32 arg = PG_GETARG_INT32(0);

    PG_RETURN_INT32(arg + 1);
}


/* by reference, variable length */

PG_FUNCTION_INFO_V1(copytext);

Datum
copytext(PG_FUNCTION_ARGS) {
    text *t = PG_GETARG_TEXT_PP(0);

    /*
     * VARSIZE_ANY_EXHDR is the size of the struct in bytes, minus the
     * VARHDRSZ or VARHDRSZ_SHORT of its header.  Construct the copy with a
     * full-length header.
     */
    text *new_t = (text *) palloc(VARSIZE_ANY_EXHDR(t) + VARHDRSZ);
    SET_VARSIZE(new_t, VARSIZE_ANY_EXHDR(t) + VARHDRSZ);

    /*
     * VARDATA is a pointer to the data region of the new struct.  The source
     * could be a short datum, so retrieve its data through VARDATA_ANY.
     */
    memcpy(VARDATA(new_t),          /* destination */
           VARDATA_ANY(t),          /* source */
           VARSIZE_ANY_EXHDR(t));   /* how many bytes */
    PG_RETURN_TEXT_P(new_t);
}

PG_FUNCTION_INFO_V1(concat_text);

Datum
concat_text(PG_FUNCTION_ARGS) {
    text *arg1 = PG_GETARG_TEXT_PP(0);
    text *arg2 = PG_GETARG_TEXT_PP(1);
    int32 arg1_size = VARSIZE_ANY_EXHDR(arg1);
    int32 arg2_size = VARSIZE_ANY_EXHDR(arg2);
    int32 new_text_size = arg1_size + arg2_size + VARHDRSZ;
    text *new_text = (text *) palloc(new_text_size);

    SET_VARSIZE(new_text, new_text_size);
    memcpy(VARDATA(new_text), VARDATA_ANY(arg1), arg1_size);
    memcpy(VARDATA(new_text) + arg1_size, VARDATA_ANY(arg2), arg2_size);
    PG_RETURN_TEXT_P(new_text);
}

PG_FUNCTION_INFO_V1(pgupper);

Datum
pgupper(PG_FUNCTION_ARGS) {
    text *t = PG_GETARG_TEXT_PP(0);

    /*
     * VARSIZE_ANY_EXHDR is the size of the struct in bytes, minus the
     * VARHDRSZ or VARHDRSZ_SHORT of its header.  Construct the copy with a
     * full-length header.
     */
    int32 len = VARSIZE_ANY_EXHDR(t);
    text *new_t = (text *) palloc(len + VARHDRSZ);
    SET_VARSIZE(new_t, len + VARHDRSZ);

    /*
     * VARDATA is a pointer to the data region of the new struct.  The source
     * could be a short datum, so retrieve its data through VARDATA_ANY.
     */
    char *src = VARDATA_ANY(t);
    char *dest = VARDATA(new_t);

    /*
     * Convert each character to uppercase
     */
    for (int i = 0; i < len; i++) {
        dest[i] = pg_toupper((unsigned char) src[i]);
    }

    PG_RETURN_TEXT_P(new_t);
}

PG_FUNCTION_INFO_V1(pyupper);


Datum
pyupper(PG_FUNCTION_ARGS) {
    text *t = PG_GETARG_TEXT_PP(0);
    PyObject *py_str = NULL, *py_upper_str = NULL;
    char *str = NULL, *upper_str = NULL;
    int32 len;

    /*
     * Get the C string from the PostgreSQL text object.
     */
    len = VARSIZE_ANY_EXHDR(t);
    str = (char *) palloc(len + 1);
    memcpy(str, VARDATA_ANY(t), len);
    str[len] = '\0';  // Null-terminate the C string

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
    char *model;
    int n; // Number of repetitions
} MyActionState;


static text *repeat_uppercase_transform(void *state, char *elem_value, int elem_len) {
    MyActionState *my_state = (MyActionState *) state; // Cast to the correct type
    int n = my_state->n; // Get the repetition count
    char *model = my_state->model;

    // Create a new text object for the transformed value
    int upper_len = elem_len * n; // Calculate the length of the resulting string
    text *result = palloc(VARHDRSZ + upper_len);
    SET_VARSIZE(result, VARHDRSZ + upper_len);

    // Fill the result with the repeated uppercase string
    char *result_ptr = VARDATA(result);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < elem_len; j++) {
            result_ptr[i * elem_len + j] = toupper(elem_value[j]);
        }
    }

    return result;
}

PG_FUNCTION_INFO_V1(jsonb_llm_generate);

Datum
jsonb_llm_generate(PG_FUNCTION_ARGS) {
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    char *model = PG_GETARG_CSTRING(1);
    JsonbIterator *it;
    JsonbParseState *parseState = NULL;
    Jsonb *res = NULL;
    JsonbValue v,
            k;
    JsonbIteratorToken type;
    bool last_was_key = false;

    MyActionState *action_state = palloc(sizeof(MyActionState));
    action_state->n = 3; // Set the number of repetitions
    action_state->model = model;

    res = transform_jsonb_string_values(
            jb,
            action_state,
            repeat_uppercase_transform // Pass the transformation function
    );

//    PG_RETURN_POINTER(JsonbValueToJsonb(res));
    PG_RETURN_JSONB_P(res);
}

//Datum
//jsonb_llm_generate(PG_FUNCTION_ARGS) {
//    Jsonb *jb = PG_GETARG_JSONB_P(0);
//    char *model = PG_GETARG_CSTRING(1);
//    JsonbIterator *it;
//    JsonbParseState *parseState = NULL;
//    JsonbValue *res = NULL;
//    JsonbValue v,
//            k;
//    JsonbIteratorToken type;
//    bool last_was_key = false;
//
//    if (JB_ROOT_IS_SCALAR(jb))
//        PG_RETURN_POINTER(jb);
//
//    it = JsonbIteratorInit(&jb->root);
//
//    while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE) {
//        Assert(!(type == WJB_KEY && last_was_key));
//
//        if (type == WJB_KEY) {
//            /* stash the key until we know if it has a null value */
//            k = v;
//            last_was_key = true;
//            continue;
//        }
//
//        if (last_was_key) {
//            /* if the last element was a key this one can't be */
//            last_was_key = false;
//
//            /* skip this field if value is null */
////            if (type == WJB_VALUE && v.type == jbvNull)
////                continue;
//
//
//            /* otherwise, do a delayed push of the key */
//            (void) pushJsonbValue(&parseState, WJB_KEY, &k);
//        }
//
//        if (type == WJB_VALUE || type == WJB_ELEM) {
//            if (v.type == jbvString) {
//                v.val.string.val = "hello";
//                v.val.string.len = strlen("hello");
//            }
//            res = pushJsonbValue(&parseState, type, &v);
//        } else
//            res = pushJsonbValue(&parseState, type, NULL);
//    }
//
//    Assert(res != NULL);
//
//    PG_RETURN_POINTER(JsonbValueToJsonb(res));
//}

PG_FUNCTION_INFO_V1(jsonb_llm_embed);

Datum jsonb_llm_embed(PG_FUNCTION_ARGS) {

    PG_RETURN_JSONB_P(PG_GETARG_JSONB_P_COPY(0));
}