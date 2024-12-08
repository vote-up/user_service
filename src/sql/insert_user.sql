INSERT INTO users_schema.users (
    phone_number,
    full_name,
    birth_date,
    gender
)
VALUES 
    ($1, $2, $3, $4::users_schema.gender)
RETURNING
    id;