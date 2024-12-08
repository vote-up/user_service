#pragma once

#include <handlers/user.pb.h>
#include <handlers/user_service.usrv.pb.hpp>
#include <string_view>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include "components/user_manager/component.hpp"

namespace handlers::user_v1_internal {
class Handler final
    : public handlers::api::users::v1::UserServiceBase::Component {
 public:
  static constexpr std::string_view kName = "handler-user-v1-internal";

  Handler(const userver::components::ComponentConfig&,
          const userver::components::ComponentContext&);

  CreateUserResult CreateUser(CallContext&,
                              api::users::v1::CreateUserRequest&&) override;

  TakeUserByPhoneNumberResult TakeUserByPhoneNumber(
      CallContext&, api::users::v1::TakeUserByPhoneNumberRequest&&) override;

  TakeUserByUUIDResult TakeUserByUUID(
      CallContext&, api::users::v1::TakeUserByUUIDRequest&&) override;

 private:
  const components::user::Component& _user_manager;
};
}  // namespace handlers::user_v1_internal