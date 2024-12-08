#include "component.hpp"
#include <boost/uuid/uuid.hpp>
#include <string_view>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/io/row_types.hpp>
#include "docs/yaml/openapi.hpp"
#include "models/user.hpp"
#include "user_service/sql_queries.hpp"
#include "userver/storages//postgres/io/date.hpp"
#include "userver/storages/postgres/cluster.hpp"
#include "userver/storages/postgres/component.hpp"
#include "userver/utils/datetime.hpp"

namespace components::user {
Component::Component(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context)
    : userver::components::LoggableComponentBase(config, component_context),
      _pg_cluster(
          component_context
              .FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()) {}

boost::uuids::uuid Component::Create(const dto::user::CreateUser& user) const {
  auto result = _pg_cluster->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      user_service::sql::kInsertUser, user.phoneNumber, user.fullName,
      user.birthDate, user.gender);

  return result.AsSingleRow<boost::uuids::uuid>();
}

dto::user::User Component::TakeUserByPhoneNumber(
    std::string_view phone_number) const {
  auto result = _pg_cluster->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      user_service::sql::kSelectUserByPhoneNumber, phone_number);

  if (result.IsEmpty()) {
    throw exceptions::NotFoundError();
  }

  return result.AsSingleRow<dto::user::User>();
}

dto::user::User Component::TakeUserByUUID(
    const boost::uuids::uuid& user_id) const {
  auto result =
      _pg_cluster->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                           user_service::sql::kSelectUserByUuid, user_id);

  if (result.IsEmpty()) {
    throw exceptions::NotFoundError();
  }

  return result.AsSingleRow<dto::user::User>(userver::storages::postgres::kRowTag);
}
}  // namespace components::user