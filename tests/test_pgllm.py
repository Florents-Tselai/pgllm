import pytest
import pathlib
import psycopg


def count_star(table: str) -> int:
    with psycopg.connect("dbname=testpgllm") as conn:
        with conn.cursor() as cur:
            cur.execute(f"select count(*) from {table}")
            return cur.fetchone()[0]


def test_populated(db):
    assert count_star("data") == 7
