#include "component.hpp"
#include <boost/uuid/uuid.hpp>
#include <string>
#include <string_view>
#include <userver/formats/parse/boost_uuid.hpp>
#include <userver/server/handlers/auth/auth_checker_base.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "components/auth_checker/utils/jwt.hpp"
#include "userver/crypto/verifiers.hpp"
#include "userver/http/common_headers.hpp"
#include "userver/http/predefined_header.hpp"

namespace components::auth_checker {
AuthCheckerBearer::AuthCheckerBearer(std::string_view access_token_public_key)
    : _access_token_public_key(access_token_public_key) {}

AuthCheckerBearer::AuthCheckResult AuthCheckerBearer::CheckAuth(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
  try {
    const auto& token =
        request.GetHeader(userver::http::headers::kAuthorization);

    if (token.empty()) {
      return AuthCheckResult{
          .status = AuthCheckResult::Status::kTokenNotFound,
          .reason = "No 'Authorization' header",
          .ext_reason = {},
          .code = userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }
    const auto token_pos = token.find(' ');

    if (token_pos == std::string::npos ||
        std::string_view{token.data(), token_pos} != "Bearer") {
      return AuthCheckResult{
          .status = AuthCheckResult::Status::kTokenNotFound,
          .reason = "'Authorization' header must match 'Bearer token'",
          .ext_reason = {},
          .code = userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }

    const userver::crypto::VerifierPs512 verifier{_access_token_public_key};

    const auto& payload =
        utils::jwt::Decode(token.substr(token_pos + 1), verifier);

    if (!payload.HasMember("sub") || !payload.HasMember("exp")) {
      return AuthCheckResult{
          .status = AuthCheckResult::Status::kInvalidToken,
          .reason = "Token must have 'sub', 'exp' fields",
          .ext_reason = {},
          .code = userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }
    if (!utils::jwt::IsPayloadValid(payload)) {
      return AuthCheckResult{
          .status = AuthCheckResult::Status::kInvalidToken,
          .reason = "Token is expired",
          .ext_reason = {},
          .code = userver::server::handlers::HandlerErrorCode::kUnauthorized};
    }
    auto user_id = payload["sub"].As<boost::uuids::uuid>();
    context.SetData("user_id", user_id);
    return {};
  } catch (const exceptions::SignatureVerificationError&) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInternalCheckFailure,
        "Deny parsing token due invalid signature",
        {},
        userver::server::handlers::HandlerErrorCode::kUnauthorized,
    };
  } catch (const exceptions::TokenParseError&) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInternalCheckFailure,
        "Unable to parse token",
        {},
        userver::server::handlers::HandlerErrorCode::kUnauthorized,
    };
  } catch (const userver::formats::json::ParseException&) {
    return AuthCheckResult{
        AuthCheckResult::Status::kInternalCheckFailure,
        "Unable to parse token payload",
        {},
        userver::server::handlers::HandlerErrorCode::kUnauthorized,
    };
  }
}
}  // namespace components::auth_checker