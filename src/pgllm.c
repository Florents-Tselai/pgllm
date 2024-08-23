#include "Python.h"

#include "postgres.h"
#include "utils/jsonb.h"
#include "utils/jsonfuncs.h"
#include "utils/builtins.h"
#include "pgllm.h"
#include "fmgr.h"


PG_MODULE_MAGIC;

void
_PG_init(void) {
    Py_Initialize();
}

/* Tear-down */
void _PG_fini(void) {
    Py_Finalize();
}

/*
 * A C-way to basically call
 *
 * 1. import llm
 * 2. py_model = llm.get_model(<model cstring>)
 * 3. response = py_model.prompt(<prompt cstring>)
 * 4. response_txt = response.text()
 *
 */
text *python_llm_generate_internal(char *prompt, int prompt_len, char *model, Jsonb *params);

text *python_llm_generate_internal(char *prompt, int prompt_len, char *model, Jsonb *params) {
    PyObject *pyllm_module = NULL,
            *pyllm_model = NULL,
            *pyllm_get_model_func = NULL,
            *pyllm_prompt_func = NULL,
            *pyllm_response = NULL,
            *pyllm_response_text_func = NULL,
            *pyllm_response_txt = NULL;

    const char *result = NULL;

    /* 1. import llm */
    pyllm_module = PyImport_ImportModule("llm");
    if (!pyllm_module) {
        PyErr_Print();
        fprintf(stderr, "Failed to load 'llm' module\n");
        return NULL;
    }

    /* 2. py_model = llm.get_model() */
    pyllm_get_model_func = PyObject_GetAttrString(pyllm_module, "get_model");
    if (!pyllm_get_model_func || !PyCallable_Check(pyllm_get_model_func)) {
        PyErr_Print();
        fprintf(stderr, "Failed to get 'get_model' function\n");
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    pyllm_model = PyObject_CallFunction(pyllm_get_model_func, "s", pyllm_model);
    Py_XDECREF(pyllm_get_model_func);
    if (!pyllm_model) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'get_model' function\n");
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    /* 3. response = py_model.prompt(<prompt cstring>) */
    pyllm_prompt_func = PyObject_GetAttrString(pyllm_model, "prompt");
    if (!pyllm_prompt_func || !PyCallable_Check(pyllm_prompt_func)) {
        PyErr_Print();
        fprintf(stderr, "Failed to get 'prompt' method\n");
        Py_XDECREF(pyllm_model);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    pyllm_response = PyObject_CallFunction(pyllm_prompt_func, "s", prompt);
    Py_XDECREF(pyllm_prompt_func); // Release reference after calling
    if (!pyllm_response) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'prompt' method\n");
        Py_XDECREF(pyllm_model);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    /* 4. response_txt = response.text() */
    pyllm_response_text_func = PyObject_GetAttrString(pyllm_response, "text");
    if (!pyllm_response_text_func || !PyCallable_Check(pyllm_response_text_func)) {
        PyErr_Print();
        fprintf(stderr, "Failed to get 'text' method\n");
        Py_XDECREF(pyllm_response);
        Py_XDECREF(pyllm_model);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    pyllm_response_txt = PyObject_CallObject(pyllm_response_text_func, NULL);
    Py_XDECREF(pyllm_response_text_func);
    if (!pyllm_response_txt) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'text' method\n");
        Py_XDECREF(pyllm_response);
        Py_XDECREF(pyllm_model);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    result = PyUnicode_AsUTF8(pyllm_response_txt);
    if (!result) {
        PyErr_Print();
        fprintf(stderr, "Failed to convert result to string\n");
        Py_XDECREF(pyllm_response);
        Py_XDECREF(pyllm_model);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    /* Cleanup */
    Py_XDECREF(pyllm_response);
    Py_XDECREF(pyllm_model);
    Py_XDECREF(pyllm_module);

    return cstring_to_text(result);
}

PG_FUNCTION_INFO_V1(pycall_model);

Datum pycall_model(PG_FUNCTION_ARGS) {
    char *prompt = text_to_cstring(PG_GETARG_TEXT_P(0));
    char *model = text_to_cstring(PG_GETARG_TEXT_P(1));
    text *result = python_llm_generate_internal(prompt, strlen(prompt), model, NULL);

    PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(pyupper);

Datum
pyupper(PG_FUNCTION_ARGS) {
    PyObject *py_str = NULL, *py_upper_str = NULL;
    char *str = text_to_cstring(PG_GETARG_TEXT_P(0));
    const char *upper_str = NULL;
    int len;


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

text *impl_pyupper(void *params, char *prompt, int prompt_len) {
    PyObject *py_str = NULL, *py_upper_str = NULL;
    const char *upper_str = NULL;
    text *result = NULL;

    py_str = PyUnicode_FromString(prompt);
    if (!py_str) {
        PyErr_Print();
        elog(ERROR, "Failed to convert C string to Python string");
    }

    /* Call the upper() method on the Python string. */
    py_upper_str = PyObject_CallMethod(py_str, "upper", NULL);
    if (!py_upper_str) {
        PyErr_Print();
        elog(ERROR, "Failed to call Python upper() method");
    }

    /* Convert the resulting Python string back to a C string. */
    upper_str = PyUnicode_AsUTF8(py_upper_str);
    if (!upper_str) {
        PyErr_Print();
        elog(ERROR, "Failed to convert Python string to C string");
    }

    result = cstring_to_text(upper_str);

    /* Clean up Python objects */
    Py_XDECREF(py_upper_str);
    Py_XDECREF(py_str);

    return result;
}

text *repeat_n_generate_internal(void *params, char *prompt, int prompt_len) {
    int n = 3;
    char *append = "\0";

    text *result = DatumGetTextPP(DirectFunctionCall2(repeat,
                                                      PointerGetDatum(cstring_to_text(prompt)),
                                                      Int32GetDatum(n)
    ));

    return result;
}

text *jsonb_transform_llm_generate(void *state, char *prompt, int prompt_len) {
    LlmModelCtxt *model = (LlmModelCtxt *) state;

    text *result = NULL;

    result = model->func.generative.generate(model->params, prompt, prompt_len);

    return result;
}

static text *jsonb_transform_llm_generate_pyllm(void *ctxt, char *prompt, int prompt_len) {
    text *result;
    LlmModelCtxt *modelCtxt = (LlmModelCtxt *) ctxt;

    result = python_llm_generate_internal(prompt, prompt_len, modelCtxt->name, modelCtxt->params);

    return result;
}


PG_FUNCTION_INFO_V1(llm_generate);

Datum
llm_generate(PG_FUNCTION_ARGS) {
    char *prompt = text_to_cstring(PG_GETARG_TEXT_PP(0));
    int32 prompt_len = strlen(prompt);
    char *model_name = text_to_cstring(PG_GETARG_TEXT_PP(1));
    Jsonb *params = NULL;
    LlmModelCtxt *modelCtxt = NULL;

    text *result = NULL;

    if (!PG_ARGISNULL(2))
        params = PG_GETARG_JSONB_P(2);

    /* search for model in the catalog */
    modelCtxt = search_models_catalog(model_name);

    if (modelCtxt) { //found in catalog
        /* The necessary context is already defined in the catalog statically, so we don't have to build it */
        result = modelCtxt->func.generative.generate(modelCtxt->params, prompt, strlen(prompt));
    } else {
        /* Model not found in static catalog, forwarding to Python LLM , but build a context first*/
        modelCtxt = (LlmModelCtxt *) palloc0(sizeof(LlmModelCtxt));
        modelCtxt->name = model_name;
//        modelCtxt->params = params;
//        modelCtxt->func.generative.generate = jsonb_transform_llm_generate_pyllm;
        result = python_llm_generate_internal(prompt, prompt_len, model_name, params);

        if (!result) {
            ereport(ERROR,
                    (errcode(ERRCODE_DATA_EXCEPTION),
                            errmsg("pgllm: something went wrong with Python LLM. Maybe %s is not supported\n",
                                   model_name)));
        }
    }

    PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(jsonb_llm_generate);

Datum
jsonb_llm_generate(PG_FUNCTION_ARGS) {
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    char *model_name = text_to_cstring(PG_GETARG_TEXT_P(1));
    Jsonb *result = NULL;
    LlmModelCtxt *model = search_models_catalog(model_name);

    if (model) {
        result = transform_jsonb_string_values(
                jb,
                model,
                jsonb_transform_llm_generate
        );
    } else {
        model = (LlmModelCtxt *) palloc(sizeof(LlmModelCtxt));
        model->name = model_name;

        result = transform_jsonb_string_values(
                jb,
                model,
                jsonb_transform_llm_generate_pyllm
        );

        pfree(model);
    }

    if (!result) {
        if (model == NULL) {
            ereport(ERROR,
                    (errcode(ERRCODE_DATA_EXCEPTION),
                            errmsg("pgllm: model %s is not supported\n", model_name)));
        }
    }

    PG_RETURN_JSONB_P(result);
}


PG_FUNCTION_INFO_V1(jsonb_llm_embed);

Datum jsonb_llm_embed(PG_FUNCTION_ARGS) {
    PG_RETURN_JSONB_P(PG_GETARG_JSONB_P_COPY(0));
}

PG_FUNCTION_INFO_V1(myjsonb_get);

Datum myjsonb_get(PG_FUNCTION_ARGS) {
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    text *key = PG_GETARG_TEXT_PP(1);

    Datum datumRes = DirectFunctionCall2(jsonb_object_field,
                                         JsonbPGetDatum(jb),
                                         PointerGetDatum(key)
    );

    PG_RETURN_DATUM(datumRes);
}
