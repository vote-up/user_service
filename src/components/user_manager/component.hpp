#pragma once

#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <string_view>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include "docs/yaml/openapi.hpp"
#include "dto/user.hpp"
#include "models/user.hpp"

namespace exceptions {
class NotFoundError : public std::runtime_error {
 public:
  NotFoundError() : std::runtime_error("Not found exception") {}
};
}  // namespace exceptions

namespace components::user {

class Component final : public userver::components::LoggableComponentBase {
 public:
  static constexpr std::string_view kName = "component-user";

  Component(const userver::components::ComponentConfig&,
            const userver::components::ComponentContext&);

  boost::uuids::uuid Create(const dto::user::CreateUser& user) const;

  dto::user::User TakeUserByPhoneNumber(std::string_view phone_number) const;

  dto::user::User TakeUserByUUID(const boost::uuids::uuid& user_id) const;

 private:
  userver::storages::postgres::ClusterPtr _pg_cluster;
};
}  // namespace components::user