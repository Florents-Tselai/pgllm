/*
 * https://chat.openai.com/share/bf7083db-0e60-4df0-b480-73a067b3c32e
 */
CREATE TABLE users
(
    id                SERIAL PRIMARY KEY,
    name              TEXT NOT NULL,
    job_title         TEXT NOT NULL,
    email             TEXT NOT NULL,
    registration_date DATE NOT NULL
);

INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Alice Johnson', 'Software Developer', 'alice.johnson@example.com', '2022-01-10');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Bob Smith', 'Project Manager', 'bob.smith@example.com', '2022-02-15');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Cathy Brown', 'Data Analyst', 'cathy.brown@example.com', '2022-03-20');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('David Lee', 'Graphic Designer', 'david.lee@example.com', '2022-04-25');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Eva Green', 'Marketing Specialist', 'eva.green@example.com', '2022-05-30');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Frank Harris', 'Sales Associate', 'frank.harris@example.com', '2022-06-05');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Grace Taylor', 'HR Manager', 'grace.taylor@example.com', '2022-07-11');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Henry White', 'Systems Administrator', 'henry.white@example.com', '2022-08-16');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Irene Scott', 'Customer Service Rep', 'irene.scott@example.com', '2022-09-21');
INSERT INTO users (name, job_title, email, registration_date)
VALUES ('Jason Martinez', 'Operations Manager', 'jason.martinez@example.com', '2022-10-26');

CREATE TABLE products
(
    id          SERIAL PRIMARY KEY,
    name        TEXT           NOT NULL,
    description TEXT           NOT NULL,
    category    TEXT           NOT NULL,
    sku         TEXT           NOT NULL,
    price       DECIMAL(10, 2) NOT NULL
);

INSERT INTO products (name, description, category, sku, price)
VALUES ('Laptop Pro', 'High performance laptop with 16GB RAM', 'Electronics', 'LP-12345', 1499.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Running Shoes', 'Comfortable running shoes for daily exercise', 'Sporting Goods', 'RS-23456', 79.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Smartphone X', 'Latest model smartphone with 128GB storage', 'Electronics', 'SX-34567', 999.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Waterproof Jacket', 'Durable waterproof jacket for all weather', 'Apparel', 'WJ-45678', 129.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Wireless Headphones', 'Noise cancelling wireless headphones', 'Electronics', 'WH-56789', 199.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Coffee Maker', 'Automatic coffee maker with multiple modes', 'Home Appliances', 'CM-67890', 89.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Yoga Mat', 'Eco-friendly and anti-slip yoga mat', 'Sporting Goods', 'YM-78901', 49.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Backpack', 'Spacious and durable backpack for travel', 'Accessories', 'BP-89012', 59.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('E-Reader', 'Lightweight e-reader with e-ink display', 'Electronics', 'ER-90123', 129.99);
INSERT INTO products (name, description, category, sku, price)
VALUES ('Table Lamp', 'LED table lamp with adjustable brightness', 'Home Decor', 'TL-01234', 39.99);

CREATE TABLE purchases
(
    id            SERIAL PRIMARY KEY,
    user_id       INT  NOT NULL,
    product_id    INT  NOT NULL,
    quantity      INT  NOT NULL,
    purchase_date DATE NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users (id),
    FOREIGN KEY (product_id) REFERENCES products (id)
);

INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (1, 3, 2, '2022-01-15');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (2, 6, 1, '2022-02-20');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (3, 2, 3, '2022-03-25');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (4, 1, 1, '2022-04-30');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (5, 5, 2, '2022-05-05');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (6, 4, 1, '2022-06-10');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (7, 7, 2, '2022-07-15');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (8, 10, 1, '2022-08-20');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (9, 8, 2, '2022-09-25');
INSERT INTO purchases (user_id, product_id, quantity, purchase_date)
VALUES (10, 9, 1, '2022-10-30');

CREATE TABLE reviews
(
    id          SERIAL PRIMARY KEY,
    user_id     INT  NOT NULL,
    product_id  INT  NOT NULL,
    rating      INT CHECK (rating BETWEEN 1 AND 5),
    review_text TEXT,
    review_date DATE NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users (id),
    FOREIGN KEY (product_id) REFERENCES products (id)
);

INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (1, 2, 4, 'Great product, but a bit expensive.', '2022-01-18');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (2, 5, 5, 'Absolutely love it!', '2022-02-22');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (3, 3, 3, 'Good, but I expected more features.', '2022-03-28');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (4, 1, 2, 'Not as described. Disappointed.', '2022-04-30');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (5, 4, 5, 'Excellent quality, worth the price.', '2022-05-06');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (6, 6, 4, 'Works well, but the setup was complicated.', '2022-06-11');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (7, 8, 1, 'Broke after a week of use.', '2022-07-16');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (8, 7, 5, 'Best purchase I ve made this year!', '2022-08-21');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (9, 9, 4, 'Pretty good, but room for improvement.', '2022-09-26');
INSERT INTO reviews (user_id, product_id, rating, review_text, review_date)
VALUES (10, 10, 3, 'Average product, nothing special.', '2022-10-31');
