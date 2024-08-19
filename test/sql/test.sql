select pyupper('hello world');

select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'repeat-1', '{}'::jsonb);

select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'repeat-3', '{"n": 3}'::jsonb);

select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'no-existing-model', '{"n": 3}'::jsonb);

select llm_generate('hello'::text, 'repeat-3'::text, '{"n": 3}'::jsonb);

select jsonb_llm_embed('"hello"'::jsonb, 'repeat');