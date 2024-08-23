-- error for model not found
select llm_generate('hello', 'non-existing-model');

-- testing a simple model coded in C and store in the static catalog
select llm_generate('hello', 'repeat-3');

--
--
--
CREATE
OR REPLACE FUNCTION is_valid_markov(generated text, prompt text)
RETURNS boolean AS $$
SELECT NOT EXISTS (SELECT word
                   FROM unnest(regexp_split_to_array(lower(generated), '\s+')) AS word
                   WHERE word <> ''
                     AND word NOT IN (SELECT unnest(regexp_split_to_array(lower(prompt), '\s+'))));
$$
LANGUAGE sql;
--
--
--

\set prompt 'The quick brown fox jumps over the lazy dog'
\set model 'markov'

-- Execute the query and store the result in a variable
SELECT llm_generate(:'prompt', :'model') AS gen_result \gset

select is_valid_markov(:'gen_result', :'prompt');
