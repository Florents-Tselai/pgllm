select jsonb_pretty('{ "firstName": "John", "lastName": "Doe",
"age": 30,
"gender": "male",
"email": "john.doe@example.com",
"phone": {
"home": "123-456-7890",
"mobile": "098-765-4321"
}}')

select jsonb_pretty(jsonb_llm_generate('{ "firstName": "John", "lastName": "Doe",
"age": 30,
"gender": "male",
"email": "john.doe@example.com",
"phone": {
"home": "123-456-7890",
"mobile": "098-765-4321"
},
"others": [{ "firstName": "John", "lastName": "Doe",
"age": 30,
"gender": "male"}]
}', 'pyupper'))