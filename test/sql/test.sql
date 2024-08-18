select add_one(1);

select copytext('hello world');

select concat_text('pa', 'sok');

select pgupper('hello world');

select pyupper('hello world');

select jsonb_llm_generate('{"message": "hello", "k1": "v1", "k2": "v2", "k3": 3}'::jsonb, 'model');

select jsonb_llm_embed('"hello"'::jsonb, 'model');