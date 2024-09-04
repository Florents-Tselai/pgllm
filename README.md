<p align="center">
<p align="center">
   <img width="50%" height="40%" src="https://raw.githubusercontent.com/Florents-Tselai/pgllm/main/docs/static/logo.webp" alt="Logo">
  </p>
  <h1 align="center">pgllm: Use LLMs in Postgres</h1>
  <p align="center">
    <a href="#api"><strong> API </strong></a> |
    <a href="#usage"><strong> Usage </strong></a> |
    <a href="#llamafile"><strong> llamafile </strong></a> |
    <a href="#embeddings"><strong> Embeddings </strong></a> |
    <a href="#installation"><strong> Installation </strong></a>
    
   </p>
<p align="center">

<p align="center">
  <a href="https://github.com/Florents-Tselai/pgllm"><img src="https://img.shields.io/badge/GitHub-repo-green"></a>
  <a href="https://github.com/Florents-Tselai/pgllm/actions/workflows/build.yml?branch=mainline"><img src="https://github.com/Florents-Tselai/pgllm/actions/workflows/build.yml/badge.svg"></a>
  <a href="http://pgllm.tselai.com/"><img src="https://readthedocs.org/projects/pgllm/badge/?version=stable"></a>
  <a href="https://www.linkedin.com/in/florentstselai/"><img src="https://img.shields.io/badge/LinkedIn-0077B5?logo=linkedin&logoColor=white"></a>
  <a href="https://github.com/sponsors/Florents-Tselai/"><img src="https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&link=https://github.com/sponsors/Florents-Tselai/"></a>
</p>

**pgllm** brings LLMs to Postgres.

It does so primarily by embedding CPython and wrapping the beautiful [llm](https://github.com/simonw/llm) Python library.

## Features

```tsql
CREATE EXTENSION pgllm;
```

Text generation with both local and remote models.

```tsql
SELECT llm_generate('hello world', 'markov', '{"length": 20, "delay": 0.2}');
SELECT llm_generate('hello world', 'mistral', '{"mistral": "abc0123"}');
```
llamafile support

```tsql
SELECT llm_generate('A story about a frog', 'llamafile')
```

Embedding models and pgvector support

```tsql
SELECT llm_embed('hello world', 'jina-embeddings-v2-small-en')::vector;
```

You can use any [LLM plugin](https://llm.datasette.io/en/stable/plugins/index.html)

## API

* `llm_generate(input text, model text[, params jsonb]) → text`
* `llm_embed(input text/bytea, model text[, params jsonb]) → float8[]`

## Usage

### Generation

Let's start by installing a simple generational model

```shell
python3 -m llm install llm-markov
```

**IMPORTANT**: 
You have to be sure that the `python3` you're using is the same one that you pointed to during the Installation;
better be explicit.

```sql
select llm_generate('hello world', 'markov');
          llm_generate          
--------------------------------
 world hello world world hello ....
(1 row)
```

### Model Parameters

Can be passed as a `jsonb` argument. 

```sql
select llm_generate('hello world', 'markov', '{"length": 20, "delay": 0.2}');
                                                       llm_generate                                                       
--------------------------------------------------------------------------------------------------------------------------
 world world hello world hello world hello world world hello world world world world world world world world world hello 
(1 row)
```

### Embeddings

Install a dummy embedding model

```shell
python3 -m llm install llm-embed-hazo
```

```sql
select llm_embed('hello world', 'hazo');

             llm_embed             
-----------------------------------
{5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
(1 row)
```

#### pgvector

If you have **pgvector** already installed 
you can cast the resulting `float8[]` to a `vector` type instead,
and use pgvector as usual.

For example to get the L2 distance:

```sql
select llm_embed('hello world', 'hazo')::vector <-> llm_embed('world hold on', 'hazo')::vector;
     ?column?     
------------------
 2.23606797749979
(1 row)
```

## Local Models

### llamafile

pgllm supports llamafile by using curl to query its web API.
This does not use the `llm-llamafile` plugin!

Install with `WITH_LLAMAFILE=1` flag

<details>
<summary>Start llamafile server</summary>

1. Download [llava-v1.5-7b-q4.llamafile](https://huggingface.co/Mozilla/llava-v1.5-7b-llamafile/resolve/main/llava-v1.5-7b-q4.llamafile?download=true) (4.29 GB).

2. Open your computer's terminal.

3. If you're using macOS, Linux, or BSD, you'll need to grant permission
for your computer to execute this new file. (You only need to do this
once.)

```sh
chmod +x llava-v1.5-7b-q4.llamafile
```

4. If you're on Windows, rename the file by adding ".exe" on the end.

5. Run the llamafile. e.g.:

```sh
./llava-v1.5-7b-q4.llamafile
```

6. Your browser should open automatically and display a chat interface.
(If it doesn't, just open your browser and point it at http://localhost:8080)

7. When you're done chatting, return to your terminal and hit
`Control-C` to shut down llamafile.

</details>

```sql
SELECT llm_generate('3 neat characteristics of a pelican', 'llamafile')::jsonb
                                                                                                                                                                                                                                                                                                   llm_generate                                                                                                                                                                                                                                                                                                   
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 {"id": "chatcmpl-qLMCLi9ghpvobVxrwr83DOVVdvaMICef", "model": "LLaMA_CPP", "usage": {"total_tokens": 132, "prompt_tokens": 58, "completion_tokens": 74}, "object": "chat.completion", "choices": [{"index": 0, "message": {"role": "assistant", "content": "1. Pelicans have a large, broad beak that is adapted for catching fish.\n2. They have a pouch under their beak, which they use to hold their catch.\n3. Pelicans are known for their distinctive wading and fishing behavior, where they stand on one leg while waiting for fish to swim by.</s>"}, "finish_reason": "stop"}], "created": 1725269144}
(1 row)
```

## Remote APIs

LLM plugins for [remote APIs](https://llm.datasette.io/en/stable/plugins/directory.html#remote-apis) 
should work easily.

Start by installing the model plugin you want, for example:

```shell
python3 -m llm install llm-mistral
```

And then use you can pass the API_KEY as a model parameter.

```shell
select llm_generate('hello world', 'mistral', '{"mistral": "abc0123"}');
```

**WARNING**:
You can easily exhaust any credits you may have by a simple `select` query.
Hence, use with caution!

## Embeddings

### JinaAI

- [`jina-embeddings-v2-small-en`](https://huggingface.co/jinaai/jina-embeddings-v2-small-en): 33 million parameters.
- [`jina-embeddings-v2-base-en`](https://huggingface.co/jinaai/jina-embeddings-v2-base-en): 137 million parameters.

```shell
python3 -m llm install llm-embed-jina
```

```sql
select llm_embed('hello world', 'jina-embeddings-v2-small-en');
```

### Onnx

```
onnx-bge-micro
onnx-gte-tiny
onnx-minilm-l6
onnx-minilm-l12
onnx-bge-small
onnx-bge-base
onnx-bge-large
```

```shell
python3 -m llm install llm-embed-onnx
```

```sql
select llm_embed('hello world', 'onnx-bge-micro');
```


## Installation

The crucial thing in the installation process is to be sure which `python3` Postgres uses.

```shell
git clone https://github.com/Florents-Tselai/pgllm.git
cd pgllm

# make sure that Python 3.XX minor versions match 
make all PYTHON=/path/to/bin/python3.11 PYTHON_CONFIG=/path/to/python3.11-config
make install
make installcheck
```

The host `postgrse` process must have access to the Python library too.
You can set LD_LIBRARY_PATH before starting postgres.

For example you may have to to something like:

```shell
export LD_LIBRARY_PATH="$pythonLocation"/lib:/usr/local/lib:/usr/lib:$HOME/local/lib:$LD_LIBRARY_PATH
$PGBIN/initdb $PGDATA
$PGBIN/pg_ctl --pgdata $PGDATA start
```

See [build.yml](./.github/workflows/build.yml)
