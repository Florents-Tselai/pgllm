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
text *python_llm_generate_internal(char *prompt, int prompt_len, char *model, char *system, bool stream,
                                   Jsonb *options);

text *python_llm_generate_internal(char *prompt, int prompt_len, char *model, char *system, bool stream,
                                   Jsonb *options) {
    PyObject *pyllm_module = NULL,
            *pyllm_model_name = NULL,
            *pyllm_system = NULL,
            *pyllm_stream = NULL,
            *pyllm_prompt_method = NULL,
            *kwoptions = NULL,
            *pyllm_response = NULL,
            *pyllm_response_txt = NULL;


    const char *result = NULL;


    /*
     * 1. import llm
     * */
    pyllm_module = PyImport_ImportModule("llm");
    if (!pyllm_module) {
        PyErr_Print();
        fprintf(stderr, "Failed to load 'llm' module\n");
        return NULL;
    }


    /* 2. py_model = llm.get_model() */
    pyllm_model_name = PyObject_CallMethod(pyllm_module, "get_model", "s", model);
    if (!pyllm_model_name) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'get_model' function\n");
        return NULL;
    }

    /*
     * 3. response = py_model.prompt(<prompt cstring>)
     *
     * prompt( self, prompt: Optional[str], system: Optional[str] = None, stream: bool = True, **options)
     * */
    pyllm_system = system ? PyUnicode_FromString(system) : Py_None;
    Py_XINCREF(pyllm_system); // Py_None should not be decremented
    pyllm_stream = PyBool_FromLong(stream);

    /* Retrieve the 'prompt' method */
    pyllm_prompt_method = PyObject_GetAttrString(pyllm_model_name, "prompt");
    if (!pyllm_prompt_method) {
        PyErr_Print();
        fprintf(stderr, "Failed to retrieve 'prompt' method\n");
        Py_XDECREF(pyllm_model_name);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    /* Create an array of arguments */
    PyObject *args[3] = {PyUnicode_FromString(prompt), pyllm_system, pyllm_stream};

    kwoptions = PyDict_New();
    if (!kwoptions) {
        PyErr_Print();
    }


    if (options) {
        // Convert options to a Python string
        PyObject *options_str = PyUnicode_FromString(JsonbToCString(NULL, &options->root, VARSIZE(options)));
        if (!options_str) {
            PyErr_Print();
        } else {
            // Import the json module
            PyObject *json_module = PyImport_ImportModule("json");
            if (!json_module) {
                PyErr_Print();
            } else {
                // Get the loads function from the json module
                PyObject *json_loads = PyObject_GetAttrString(json_module, "loads");
                if (!json_loads || !PyCallable_Check(json_loads)) {
                    PyErr_Print();
                } else {
                    // Call json.loads(options_str)
                    PyObject *parsed_dict = PyObject_CallFunctionObjArgs(json_loads, options_str, NULL);
                    if (!parsed_dict) {
                        PyErr_Print();
                    } else if (PyDict_Check(parsed_dict)) {
                        // Set kwoptions to the parsed dictionary
                        PyDict_Update(kwoptions, parsed_dict);
                    }
                    Py_XDECREF(parsed_dict);
                }
                Py_XDECREF(json_loads);
            }
            Py_XDECREF(json_module);
        }
        Py_XDECREF(options_str);
    }

    /* Call the method using PyObject_VectorcallDict */
    pyllm_response = PyObject_VectorcallDict(pyllm_prompt_method, args, 3 | PY_VECTORCALL_ARGUMENTS_OFFSET,
                                             kwoptions);
    if (!pyllm_response) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'prompt' method\n");
        Py_XDECREF(pyllm_system);
        Py_XDECREF(pyllm_stream);
        Py_XDECREF(pyllm_prompt_method);
        Py_XDECREF(pyllm_model_name);
        Py_XDECREF(pyllm_module);
        return NULL;
    }

    /* 4. response_txt = response.text() */

    pyllm_response_txt = PyObject_CallMethod(pyllm_response, "text", NULL);
    if (!pyllm_response_txt) {
        PyErr_Print();
        fprintf(stderr, "Failed to call 'text' method\n");
        return NULL;
    }

    result = PyUnicode_AsUTF8(pyllm_response_txt);
    if (!result) {
        PyErr_Print();
        fprintf(stderr, "Failed to convert result to string\n");
        return NULL;
    }

    /* Cleanup */
    Py_XDECREF(pyllm_response);
    Py_XDECREF(pyllm_model_name);
    Py_XDECREF(pyllm_module);

    return cstring_to_text(result);
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

    result = python_llm_generate_internal(prompt, prompt_len, modelCtxt->name, "", 0, modelCtxt->params);

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

    if (modelCtxt) {
        //found in catalog
        /* The necessary context is already defined in the catalog statically, so we don't have to build it */
        result = modelCtxt->func.generative.generate(modelCtxt->params, prompt, strlen(prompt));
    } else {
        /* Model not found in static catalog, forwarding to Python LLM , but build a context first*/
        modelCtxt = (LlmModelCtxt *) palloc0(sizeof(LlmModelCtxt));
        modelCtxt->name = model_name;
        modelCtxt->params = params;
        result = python_llm_generate_internal(prompt, prompt_len, model_name, "", 0, params);

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
