#pragma once

#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/utils/strong_typedef.hpp>

namespace auth_checker::config {
using AccessTokenPrivateKey =
    userver::utils::NonLoggable<class AccessTokenPrivateKeyTag, std::string>;

struct Secrets {
  explicit Secrets(const userver::formats::json::Value& doc);

  std::string access_token_public_key;
  AccessTokenPrivateKey access_token_private_key;
};
}  // namespace components::token::secdist