#pragma once

#include <string>
#include <userver/server/handlers/auth/auth_checker_base.hpp>

namespace components::auth_checker {
class AuthCheckerBearer final
    : public userver::server::handlers::auth::AuthCheckerBase {
 public:
  using AuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  AuthCheckerBearer(std::string_view);

  [[nodiscard]] virtual AuthCheckResult CheckAuth(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

  [[nodiscard]] virtual bool SupportsUserAuth() const noexcept override {
    return true;
  }

 private:
  std::string _access_token_public_key;
};
}  // namespace components::auth_checker