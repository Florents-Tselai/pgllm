# pgllm: Use LLMs in Postgres

[![Github](https://img.shields.io/static/v1?label=GitHub&message=Repo&logo=GitHub&color=green)](https://github.com/Florents-Tselai/pgllm)
[![Documentation Status](https://readthedocs.org/projects/pgllm/badge/?version=stable)](http://pgllm.tselai.com/en/latest/?badge=stable)
[![Linkedin](https://img.shields.io/badge/LinkedIn-0077B5?logo=linkedin&logoColor=white)](https://www.linkedin.com/in/florentstselai/)
[![Github Sponsors](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&color=pink)](https://github.com/sponsors/Florents-Tselai/)
[![Build](https://github.com/Florents-Tselai/pgllm/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Florents-Tselai/pgllm/actions?query=workflow%3Abuild)
[![License](https://img.shields.io/badge/BSD%20license-blue.svg)](https://github.com/Florents-Tselai/pgllm/blob/main/LICENSE)

**pgllm** brings LLMs to Postgres, by embedding the Python [llm](github.com/simonw/llm) library.

## API

* `llm_generate(input text, model text[, params jsonb]) → text`
* `llm_embed(input text/bytea, model text[, params jsonb]) → float8[]`
  
## Features

- **Text Generation:** Generate text based on input prompts using specified LLM models.
- **Text/Binary Embedding:** Convert text into numerical embeddings for use in machine learning models or similarity searches.
- **Customizable Parameters:** Pass additional parameters to the LLM models as JSONB for more flexible text generation and embedding.
- **pgvector** integration
- Support for [LLM plugins](https://llm.datasette.io/en/stable/plugins/index.html)

## Installing Models

You have to be sure that the `python3` you're using is the same one that you pointed to during the Installation.

Some dummy models 
```shell
python3 -m llm install llm-markov
python3 -m llm install llm-embed-hazo
```

Some more sophisticated models that will download artifacts in the background, the first time they'll be used 

```shell
python3 -m llm install llm-embed-jina
python3 -m llm install llm-embed-onnx
```

## Embeddings

```sql
select llm_embed('hello world', 'hazo');
             llm_embed             
-----------------------------------
 {5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
(1 row)
```

## Generation

```sql
select llm_generate('hello world', 'markov');
          llm_generate          
--------------------------------
 world hello world world hello ....
(1 row)
```


### Model Parameteres

Can be passed as a `jsonb` argument. 

```sql
select llm_generate('hello world', 'markov', '{"length": 20, "delay": 0.2}');
                                                       llm_generate                                                       
--------------------------------------------------------------------------------------------------------------------------
 world world hello world hello world hello world world hello world world world world world world world world world hello 
(1 row)
```

## Installation

See [build.yml](workflows/.github/build.yml)
