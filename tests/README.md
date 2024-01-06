```sh
cat quotes.csv | psql -d testpgllm -c "COPY quotes(author, quote) FROM stdin WITH (FORMAT csv, HEADER);"
```
