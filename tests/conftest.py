import pytest
import pathlib
import psycopg
from pgllm.db import Database

sql_file = pathlib.Path(__file__).parent / "data.sql"


@pytest.fixture
def db():
    conninfo = "dbname=testpgllm"
    with open(sql_file, 'r') as f:
        with psycopg.connect(conninfo) as conn:
            with conn.cursor() as cur:
                cur.execute(f.read())
            conn.commit()

    return Database(conninfo)
