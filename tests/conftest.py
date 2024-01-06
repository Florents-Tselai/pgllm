import pytest
import pathlib
from pgapi import Database
from pgapi.utils import sqlite3
import pytest
import psycopg
import os
import llm
import pytest
import sqlite_utils
import json
import llm
from llm.plugins import pm
from pydantic import Field
from typing import Optional


def pytest_configure(config):
    import sys

    sys._called_from_test = True


@pytest.fixture
def embed_demo():
    return EmbedDemo()


DB_TEST_NAME = "testpgllm"


class EmbedDemo(llm.EmbeddingModel):
    model_id = "embed-demo"
    batch_size = 10
    supports_binary = True

    def __init__(self):
        self.embedded_content = []

    def embed_batch(self, texts):
        if not hasattr(self, "batch_count"):
            self.batch_count = 0
        self.batch_count += 1
        for text in texts:
            self.embedded_content.append(text)
            words = text.split()[:16]
            embedding = [len(word) for word in words]
            # Pad with 0 up to 16 words
            embedding += [0] * (16 - len(embedding))
            yield embedding


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


class MockModel(llm.Model):
    model_id = "mock"

    class Options(llm.Options):
        max_tokens: Optional[int] = Field(
            description="Maximum number of tokens to generate.", default=None
        )

    def __init__(self):
        self.history = []
        self._queue = []

    def enqueue(self, messages):
        assert isinstance(messages, list)
        self._queue.append(messages)

    def execute(self, prompt, stream, response, conversation):
        self.history.append((prompt, stream, response, conversation))
        while True:
            try:
                messages = self._queue.pop(0)
                yield from messages
                break
            except IndexError:
                break


class EmbedDemo(llm.EmbeddingModel):
    model_id = "embed-demo"
    batch_size = 10
    supports_binary = True

    def __init__(self):
        self.embedded_content = []

    def embed_batch(self, texts):
        if not hasattr(self, "batch_count"):
            self.batch_count = 0
        self.batch_count += 1
        for text in texts:
            self.embedded_content.append(text)
            words = text.split()[:16]
            embedding = [len(word) for word in words]
            # Pad with 0 up to 16 words
            embedding += [0] * (16 - len(embedding))
            yield embedding


class EmbedBinaryOnly(EmbedDemo):
    model_id = "embed-binary-only"
    supports_text = False
    supports_binary = True


class EmbedTextOnly(EmbedDemo):
    model_id = "embed-text-only"
    supports_text = True
    supports_binary = False


@pytest.fixture
def embed_demo():
    return EmbedDemo()


@pytest.fixture
def mock_model():
    return MockModel()


@pytest.fixture(autouse=True)
def register_embed_demo_model(embed_demo, mock_model):
    class MockModelsPlugin:
        __name__ = "MockModelsPlugin"

        @llm.hookimpl
        def register_embedding_models(self, register):
            register(embed_demo)
            register(EmbedBinaryOnly())
            register(EmbedTextOnly())

        @llm.hookimpl
        def register_models(self, register):
            register(mock_model)

    pm.register(MockModelsPlugin(), name="undo-mock-models-plugin")
    try:
        yield
    finally:
        pm.unregister(name="undo-mock-models-plugin")
