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

SELECT llm_generate(:'prompt', :'model') AS gen_result \gset

select is_valid_markov(:'gen_result', :'prompt');
-- END DEST CASE

-- BEGIN TEST CASE: markov with length paraμ
SELECT llm_generate(:'prompt', :'model') AS gen_result \gset

select is_valid_markov(:'gen_result', :'prompt');
-- END TEST CASE