DROP TABLE IF EXISTS DATA CASCADE ;

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

create view dummy as
select *
from data;
