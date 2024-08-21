CREATE OR REPLACE FUNCTION is_valid_markov(prompt TEXT, generated TEXT)
RETURNS BOOLEAN AS $$
DECLARE
prompt_words TEXT[];
    generated_words TEXT[];
    word TEXT;
    is_contained BOOLEAN := TRUE;
BEGIN
    -- Split the prompt and generated text into arrays of words
    prompt_words := string_to_array(lower(prompt), ' ');
    generated_words := string_to_array(lower(generated), ' ');

    -- Check if every word in prompt_words is contained in generated_words
    FOREACH word IN ARRAY prompt_words
    LOOP
        IF NOT word = ANY(generated_words) THEN
            is_contained := FALSE;
            EXIT;  -- Exit the loop early if a word is not found
END IF;
END LOOP;

RETURN is_contained;
END;
$$ LANGUAGE plpgsql;




-- select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'no-existing-model', '{"n": 3}'::jsonb);



select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'repeat-3', '{"n": 3}'::jsonb);

select jsonb_llm_generate('{"message": "hello", "a": [1, "one", 2, 3]}'::jsonb, 'pyupper', '{"n": 3}'::jsonb);

select jsonb_llm_generate('{"message": "hello"}'::jsonb, 'pyupper', '{"n": 3}'::jsonb);

select is_valid_markov('hello there', llm_generate('hello there', 'markov'));

select llm_generate('hello'::text, 'repeat-3'::text, '{"n": 3}'::jsonb);

select jsonb_llm_embed('"hello"'::jsonb, 'repeat');

