#pragma once

#include <string>
namespace dto {
struct ParsedToken {
  std::string header;
  std::string payload;
  std::string signature;
};
}  // namespace dto