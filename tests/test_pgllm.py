import pytest
import pathlib
import psycopg


def test_populated(setup_db):
    with psycopg.connect("dbname=testpgllm") as conn:
        with conn.cursor() as cur:
            cur.execute("select count(*) from data")
            assert cur.fetchone()[0] == 7
