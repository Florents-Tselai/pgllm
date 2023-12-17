DROP TABLE IF EXISTS DATA;

CREATE TABLE data
(
    id serial primary key,
    c1 text
);

INSERT INTO data(c1)
VALUES ('UA502'),
       ('Bananas'),
       ('Comedy'),
       ('82 minutes'),
       ('T_601'),
       ('Yojimbo'),
       ('Drama');
