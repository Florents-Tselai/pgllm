import pytest
import pathlib
import psycopg

sql_file = pathlib.Path(__file__).parent / "data.sql"


@pytest.fixture
def setup_db():
    with open(sql_file, 'r') as f:
        with psycopg.connect("dbname=testpgllm") as conn:
            with conn.cursor() as cur:
                cur.execute(f.read())
            conn.commit()
