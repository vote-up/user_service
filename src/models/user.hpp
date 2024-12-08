#pragma once

#include <boost/uuid/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime/date.hpp>
#include <userver/utils/trivial_map.hpp>
#include "userver/storages/postgres/io/enum_types.hpp"

namespace models {

enum class Gender { kMale, kFemale };

constexpr userver::utils::TrivialBiMap kGenderToStr = [](auto selector) {
  return selector().Case(Gender::kMale, "male").Case(Gender::kFemale, "female");
};

constexpr userver::utils::TrivialBiMap kStrToGender = [](auto selector) {
  return selector().Case("male", Gender::kMale).Case("female", Gender::kFemale);
};

struct User {
  boost::uuids::uuid id;
  userver::storages::postgres::TimePointTz created_at;
  std::string phone_number;
  std::string full_name;
  userver::utils::datetime::Date birth_date;
  Gender gender;
};
}  // namespace models

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<models::Gender> : EnumMappingBase<models::Gender> {
  static constexpr DBTypeName postgres_name = "users_schema.gender";
  static constexpr userver::utils::TrivialBiMap enumerators =
      models::kGenderToStr;
};
}  // namespace userver::storages::postgres::io