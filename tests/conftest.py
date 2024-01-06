import pytest
import pathlib
from pgapi import Database
from pgapi.utils import sqlite3
import pytest
import psycopg
import os

DB_TEST_NAME = "testpgllm"


def pytest_configure(config):
    import sys

    sys._called_from_test = True


@pytest.fixture
def load_file():
    def _load_file(filename):
        base_dir = os.path.dirname(
            __file__
        )  # Gets the directory where the test file is located
        file_path = os.path.join(
            base_dir, "data", filename
        )  # Adjust the path to your file structure
        with open(file_path, "r") as file:
            return file.read()

    return _load_file


@pytest.fixture
def fresh_db():
    with psycopg.connect(f"dbname=postgres", autocommit=True) as conn:
        conn.execute(f"DROP DATABASE IF EXISTS {DB_TEST_NAME}")
        conn.execute(f"CREATE DATABASE {DB_TEST_NAME}")
    db = Database(f"dbname={DB_TEST_NAME}")
    return db


@pytest.fixture
def db(fresh_db, load_file):
    sql_script = load_file("eshop.sql") + load_file("quotes.sql")
    fresh_db.execute(sql_script)
    return fresh_db
