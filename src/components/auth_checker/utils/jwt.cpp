#include "jwt.hpp"
#include <fmt/format.h>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <userver/crypto/base64.hpp>
#include <userver/crypto/exception.hpp>
#include <userver/crypto/signers.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include "components/auth_checker/dto/token.hpp"

namespace utils::jwt {

namespace {
dto::ParsedToken ParseToken(std::string_view token) {
  dto::ParsedToken parsed_token;

  auto header_end = token.find('.');
  if (header_end == std::string::npos) {
    throw exceptions::TokenParseError(
        std::format("invalid token - no payload '{}'", token));
  }
  parsed_token.header = std::string{token.substr(0, header_end)};
  token.remove_prefix(header_end + 1);

  auto payload_end = token.find('.');
  if (payload_end == std::string::npos) {
    throw exceptions::TokenParseError(std::format(
        "invalid token - no signature '{}{}'", parsed_token.header, token));
  }
  parsed_token.header = std::string{token.substr(0, payload_end)};
  token.remove_prefix(payload_end + 1);

  if (token.find('.') != std::string_view::npos) {
    throw exceptions::TokenParseError(
        std::format("unsupported encrypted token - '{}.{}.{}'",
                    parsed_token.header, parsed_token.payload, token));
  }

  parsed_token.signature = std::string(token);

  return parsed_token;
}

userver::formats::json::Value DecodeTokenPart(
    std::string_view encoded_token_part, std::string_view part_name) {
  try {
    return userver::formats::json::FromString(
        userver::crypto::base64::Base64UrlDecode(encoded_token_part));
  } catch (const userver::crypto::CryptoException&) {
    throw exceptions::TokenParseError(
        std::format("Failed to decode JWT '{}' part - '{}'", part_name,
                    encoded_token_part));
  } catch (const userver::formats::json::Exception&) {
    throw exceptions::TokenParseError(std::format(
        "Failed to parse JWT '{}' part - '{}'", part_name, encoded_token_part));
  }
}
}  // namespace

userver::formats::json::Value Decode(
    const std::string& token, const userver::crypto::Verifier& verifier) {
  const auto parsed_token = ParseToken(token);

  const auto header = DecodeTokenPart(parsed_token.header, "header");

  // retrieve sign algo from header
  std::string algo;
  try {
    algo = header["alg"].As<std::string>();
  } catch (const userver::formats::json::Exception&) {
    throw exceptions::TokenParseError(
        std::format("JWT header alg field missing or malformed - '{}'",
                    userver::formats::json::ToString(header)));
  }

  auto payload = DecodeTokenPart(parsed_token.payload, "payload");

  try {
    // verify if we have appropriate signer
    if (verifier.Name() != algo) {
      throw exceptions::SignatureVerificationError(fmt::format(
          "JWT encoded with alg '{}' and verifier '{}' was provided", algo,
          verifier.Name()));
    }

    // verify signature
    verifier.Verify(
        {parsed_token.header, ".", parsed_token.payload},
        userver::crypto::base64::Base64UrlDecode(parsed_token.signature));
  } catch (const userver::crypto::CryptoException&) {
    throw exceptions::TokenParseError(
        std::format("Failed to Base64-decode JWT signature - '{}'",
                    parsed_token.signature));
  }
  return payload;
}

bool IsPayloadValid(const userver::formats::json::Value& payload,
                    std::chrono::system_clock::time_point now) {
  const auto exp = payload["exp"].As<std::optional<int64_t>>();
  const auto now_unix = std::chrono::system_clock::to_time_t(now);
  if (exp && exp.value() < now_unix) {
    return false;
  }
  return true;
}
}  // namespace utils::jwt