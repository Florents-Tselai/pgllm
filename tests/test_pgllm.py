import pytest


def test_dummy(db):
    "Assert that create tables have the expected num of rows"
    assert list(db.query("select count(*) from quotes")) == [{"count": 1664}]
    for c in ["products", "purchases", "reviews", "users"]:
        assert list(db.query(f"select count(*) from {c}")) == [{"count": 10}]
