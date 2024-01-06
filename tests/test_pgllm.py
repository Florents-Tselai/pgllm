import pytest
import llm
from llm.embeddings import Entry


def test_dummy(db):
    "Assert that create tables have the expected num of rows"
    assert list(db.query("select count(*) from quotes")) == [{"count": 1664}]
    for c in ["products", "purchases", "reviews", "users"]:
        assert list(db.query(f"select count(*) from {c}")) == [{"count": 10}]


def test_demo_plugin():
    # sentence-transformers/all-MiniLM-L6-v2
    model = llm.get_embedding_model("embed-demo")
    assert model.embed("hello world") == [5, 5] + [0] * 14
