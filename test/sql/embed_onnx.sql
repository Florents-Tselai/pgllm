select array_length(llm_embed('hello world', 'onnx-bge-micro'), 1);
select array_length(llm_embed('hello world', 'onnx-gte-tiny'), 1);
select array_length(llm_embed('hello world', 'onnx-minilm-l6'), 1);
select array_length(llm_embed('hello world', 'onnx-minilm-l12'), 1);
