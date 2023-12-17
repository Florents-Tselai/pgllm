from pgllm import Database


def test_db_api(db):
    assert db.name == 'testpgllm'
    assert db.execute("select * from data").fetchall() == [(1, 'UA502'),
                                                           (2, 'Bananas'),
                                                           (3, 'Comedy'),
                                                           (4, '82 minutes'),
                                                           (5, 'T_601'),
                                                           (6, 'Yojimbo'),
                                                           (7, 'Drama')]

    assert list(db.query("select * from data")) == [{'c1': 'UA502', 'id': 1},
                                                    {'c1': 'Bananas', 'id': 2},
                                                    {'c1': 'Comedy', 'id': 3},
                                                    {'c1': '82 minutes', 'id': 4},
                                                    {'c1': 'T_601', 'id': 5},
                                                    {'c1': 'Yojimbo', 'id': 6},
                                                    {'c1': 'Drama', 'id': 7}]
