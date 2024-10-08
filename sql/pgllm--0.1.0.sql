CREATE FUNCTION llm_generate(text, text, jsonb default null) RETURNS text
AS
'MODULE_PATHNAME'
    LANGUAGE C PARALLEL SAFE;

CREATE FUNCTION llm_embed(text, text, jsonb default '{}') RETURNS float8[]
AS
'MODULE_PATHNAME'
    LANGUAGE C PARALLEL SAFE;

CREATE FUNCTION jsonb_llm_generate(jsonb, text, jsonb default null) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION jsonb_llm_embed(jsonb, text) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;
