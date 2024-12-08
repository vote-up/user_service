#include "component.hpp"
#include <memory>
#include <userver/storages/secdist/component.hpp>
#include "components/auth_checker/auth_checker_bearer/component.hpp"
#include "components/auth_checker/auth_checker_bearer_factory/secdist_config.hpp"

namespace components::auth_checker {
userver::server::handlers::auth::AuthCheckerBasePtr
AuthCheckerBearerFactory::operator()(
    const userver::components::ComponentContext& component_context,
    const userver::server::handlers::auth::HandlerAuthConfig& auth_config,
    const userver::server::handlers::auth::AuthCheckerSettings&) const {
  const auto& secdist =
      component_context.FindComponent<userver::components::Secdist>()
          .Get()
          .Get<::auth_checker::config::Secrets>();
  const auto& access_token_public_key = secdist.access_token_public_key;

  return std::make_shared<AuthCheckerBearer>(access_token_public_key);
}
}  // namespace components::auth_checker