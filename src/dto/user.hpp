#pragma once

#include <string>
#include <userver/formats/parse/to.hpp>
#include <userver/utils/datetime/date.hpp>
#include "models/user.hpp"
namespace dto {

struct User {
  std::string phone_number;
  std::string full_name;
  userver::utils::datetime::Date birth_date;
  models::Gender gender;
};
}  // namespace dto