DROP SCHEMA IF EXISTS users_schema CASCADE;

CREATE SCHEMA IF NOT EXISTS users_schema;

CREATE TYPE users_schema.gender AS ENUM (
    'male',
    'female'
);

CREATE TABLE IF NOT EXISTS users_schema.users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    phone_number VARCHAR(15) UNIQUE NOT NULL,
    full_name TEXT NOT NULL,
    birth_date DATE NOT NULL,
    gender users_schema.gender NOT NULL
);