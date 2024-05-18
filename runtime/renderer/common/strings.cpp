#include "strings.h"

#include <sstream>

namespace vkb {
std::vector<std::string> split(const std::string &input, char delim) {
  std::vector<std::string> tokens;

  std::stringstream sstream(input);
  std::string token;
  while (std::getline(sstream, token, delim)) {
    tokens.push_back(token);
  }

  return tokens;
}
}  // namespace vkb