#include "handler.hpp"
#include <grpcpp/support/status.h>
#include <handlers/user.pb.h>
#include <handlers/user_service.usrv.pb.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/exceptions.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime/date.hpp>
#include "components/user_manager/component.hpp"
#include "docs/yaml/openapi.hpp"
#include "models/user.hpp"

namespace handlers::user_v1_internal {

namespace {
std::string GenderToStr(api::users::v1::Gender gender) {
  switch (gender) {
    case api::users::v1::MALE:
      return "male";
    case api::users::v1::FEMALE:
      return "female";
    default:
      throw std::runtime_error("WTF");
  }
}

api::users::v1::User MakeUser(const dto::user::User& user) {
  api::users::v1::User proto_user;
  api::users::v1::Gender gender =
      user.gender == "male" ? api::users::v1::MALE : api::users::v1::FEMALE;
  proto_user.set_id(userver::utils::ToString(user.id));
  proto_user.set_full_name(user.fullName);
  proto_user.set_phone_number(user.phoneNumber);
  proto_user.set_birth_date(
      userver::utils::datetime::ToString(user.birthDate));
  proto_user.set_gender(gender);
  return proto_user;
}

}  // namespace

Handler::Handler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& component_context)
    : handlers::api::users::v1::UserServiceBase::Component(config,
                                                           component_context),
      _user_manager(
          component_context.FindComponent<components::user::Component>()) {}

Handler::CreateUserResult Handler::CreateUser(
    CallContext&, api::users::v1::CreateUserRequest&& request) {
  try {
    api::users::v1::CreateUserResponse response;

    dto::user::CreateUser user{
        .phoneNumber = request.phone_number(),
        .fullName = request.full_name(),
        .birthDate = userver::utils::datetime::DateFromRFC3339String(
            request.birth_date()),
        .gender = GenderToStr(request.gender())};

    const auto& user_id = _user_manager.Create(user);
    response.set_user_id(userver::utils::ToString(user_id));
    return response;
  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::ALREADY_EXISTS, ex.what());
  } catch (const std::runtime_error& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::INVALID_ARGUMENT, ex.what());
  }
}

Handler::TakeUserByPhoneNumberResult Handler::TakeUserByPhoneNumber(
    CallContext&, api::users::v1::TakeUserByPhoneNumberRequest&& request) {
  try {
    api::users::v1::User response;

    const auto& user =
        _user_manager.TakeUserByPhoneNumber(request.phone_number());

    return MakeUser(user);
  } catch (const exceptions::NotFoundError& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::NOT_FOUND, ex.what());
  } catch (const std::runtime_error& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::INVALID_ARGUMENT, ex.what());
  }
}

Handler::TakeUserByUUIDResult Handler::TakeUserByUUID(
    CallContext&, api::users::v1::TakeUserByUUIDRequest&& request) {
  try {
    api::users::v1::User response;

    const auto& user = _user_manager.TakeUserByUUID(
        userver::utils::BoostUuidFromString(request.user_id()));

    return MakeUser(user);
  } catch (const exceptions::NotFoundError& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::NOT_FOUND, ex.what());
  } catch (const std::runtime_error& ex) {
    LOG_WARNING() << ex.what();
    return grpc::Status(grpc::INVALID_ARGUMENT, ex.what());
  }
}
}  // namespace handlers::user_v1_internal