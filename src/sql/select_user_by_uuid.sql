SELECT
    id,
    created_at,
    phone_number,
    full_name,
    birth_date,
    gender
FROM
    users_schema.users
WHERE
    id = $1