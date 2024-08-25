CREATE FUNCTION llm_generate(text, text, jsonb default null) RETURNS text
AS
'MODULE_PATHNAME'LANGUAGE C;

CREATE FUNCTION llm_embed(text, text, jsonb default '{}') RETURNS float8[]
AS
'MODULE_PATHNAME',
'llm_embed_text'
    LANGUAGE C;

CREATE FUNCTION llm_embed(bytea, text, jsonb default '{}') RETURNS float8[]
AS
'MODULE_PATHNAME',
'llm_embed_binary'
    LANGUAGE C;

CREATE FUNCTION jsonb_llm_generate(jsonb, text, jsonb default null) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;

CREATE FUNCTION jsonb_llm_embed(jsonb, text) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C;
