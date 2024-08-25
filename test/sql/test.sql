-- testing a simple model coded in C and store in the static catalog
select llm_generate('hello', 'repeat-3');

--
--
--
CREATE OR REPLACE FUNCTION is_valid_markov(generated_ text, prompt text, length_ int default null)
RETURNS boolean AS $$
BEGIN
    -- Optional: Check the length of the generated text
    IF length_ IS NOT NULL THEN
        IF length(generated) <> length_ THEN
            RETURN false;
END IF;
END IF;

    -- Check if all words in 'generated' are contained in 'prompt'
RETURN NOT EXISTS (
    SELECT word
    FROM unnest(regexp_split_to_array(lower(trim(generated_)), '\s+')) AS word
    WHERE word <> ''
      AND word NOT IN (
        SELECT unnest(regexp_split_to_array(lower(trim(generated_)), '\s+'))
    )
);
END;
$$
LANGUAGE plpgsql;

--
--
--

-- BEGIN TEST CASE: markov without parameters
\set prompt 'The quick brown fox jumps over the lazy dog'
\set model 'markov'
--see https://github.com/simonw/llm-markov/blob/657ca504bcf9f0bfc1c6ee5fe838cde9a8976381/llm_markov/__init__.py#L22
\set params '{"length": 20, "delay": 0.2}'

SELECT llm_generate(:'prompt', :'model') AS gen_result \gset

select is_valid_markov(:'gen_result', :'prompt');
-- END DEST CASE

-- BEGIN TEST CASE: markov with params
SELECT llm_generate(:'prompt', :'model', :'params') AS gen_result \gset

select is_valid_markov(:'gen_result', :'prompt');
select array_length(regexp_split_to_array(:'gen_result', '\s+'), 1) - 1; --FIXME: this returns length + 1 (probably whitespace)
-- END TEST CASE

--
--
--
select llm_embed('hello world', 'hazo');
select llm_embed(:'prompt', 'hazo');

select array_length(llm_embed('hello world', 'jina-embeddings-v2-small-en'), 1);
select array_length(llm_embed(:'prompt', 'jina-embeddings-v2-small-en'), 1);

select array_length(llm_embed('hello world', 'onnx-bge-micro'), 1);
select array_length(llm_embed(:'prompt', 'onnx-bge-micro'), 1);


