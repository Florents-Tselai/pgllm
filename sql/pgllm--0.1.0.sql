CREATE FUNCTION pyupper(text) RETURNS text
AS
'MODULE_PATHNAME'
    LANGUAGE C STRICT;

CREATE FUNCTION jsonb_llm_generate(jsonb, text, jsonb default '{}') RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C STRICT;

CREATE FUNCTION llm_generate(text, text, jsonb default '{}') RETURNS text
AS
'
    select jsonb_llm_generate(to_jsonb($1), $2, $3)::text
' language sql;

CREATE FUNCTION jsonb_llm_embed(jsonb, text) RETURNS jsonb
AS
'MODULE_PATHNAME'
    LANGUAGE C STRICT;