CREATE TABLE prompts
(
    id     SERIAL PRIMARY KEY,
    prompt TEXT NOT NULL
);

INSERT INTO prompts (prompt)
VALUES ('What is your favorite book and why?'),
       ('Describe a time when you overcame a challenge.'),
       ('If you could travel anywhere in the world, where would you go and why?'),
       ('What are your top three goals for the next year?'),
       ('Explain a concept you find fascinating and why.'),
       ('What does success mean to you?'),
       ('Describe a memorable moment from your childhood.'),
       ('What is a skill you want to learn and why?'),
       ('If you could meet any historical figure, who would it be and why?'),
       ('What are the qualities of a great leader in your opinion?');
