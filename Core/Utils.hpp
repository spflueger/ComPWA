#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace ComPWA {
namespace Utils {

/// Check of numbers \p x and \p are equal within \p nEpsion times the numerical
/// limit.
inline bool equal(double x, double y, int nEpsilon) {
  return std::abs(x - y) < std::numeric_limits<double>::epsilon() *
                               std::abs(x + y) * nEpsilon ||
         std::abs(x - y) < std::numeric_limits<double>::min();
}

/// split the string into pieces, which are separated by the separator
/// character (default separator: space)
inline std::vector<std::string> splitString(const std::string &str,
                                            char separator = ' ') {
  std::vector<std::string> result;
  std::istringstream ss(str);
  std::string token;

  while (std::getline(ss, token, separator)) {
    result.push_back(token);
  }
  return result;
}

} // namespace Utils
} // namespace ComPWA
