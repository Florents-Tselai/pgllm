CREATE FUNCTION pyupper(text) RETURNS text
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION llm_generate(text, text, jsonb default null) RETURNS text
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION jsonb_llm_generate(jsonb, text, jsonb default null) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION jsonb_llm_embed(jsonb, text) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION pycall_model(text, text) returns text AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION myjsonb_get(jsonb, text) returns text AS
'MODULE_PATHNAME'
    LANGUAGE C;