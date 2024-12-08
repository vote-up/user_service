#pragma once

#include <chrono>
#include <stdexcept>
#include <system_error>
#include <userver/crypto/base64.hpp>
#include <userver/crypto/exception.hpp>
#include <userver/crypto/signers.hpp>
#include <userver/crypto/verifiers.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/utils/datetime.hpp>

namespace exceptions {
class TokenParseError : public userver::crypto::CryptoException {
 public:
  using CryptoException::CryptoException;
};

class SignatureVerificationError : public userver::crypto::VerificationError {
 public:
  using VerificationError::VerificationError;
};

class ExpiredToken : public std::runtime_error {
 public:
  ExpiredToken() : std::runtime_error("expired token") {}
};
}  // namespace exceptions

namespace utils::jwt {

userver::formats::json::Value Decode(const std::string& token,
                                     const userver::crypto::Verifier& verifier);

bool IsPayloadValid(const userver::formats::json::Value& payload,
                    std::chrono::system_clock::time_point now =
                        userver::utils::datetime::Now());
}  // namespace utils::jwt