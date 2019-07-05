#ifndef CORE_FITPARAMETER_HPP_
#define CORE_FITPARAMETER_HPP_

#include <string>
#include <vector>

namespace ComPWA {

template <typename T> struct FitParameter {
  FitParameter() = default;
  FitParameter(std::string name, T val, T min, T max)
      : HasBounds(true), IsFixed(true), Value(val), Name(name),
        Bounds(min, max) {}
  bool HasBounds = false;
  bool IsFixed = false;
  T Value;
  std::string Name = "";
  std::pair<T, T> Bounds;
};

using FitParameterList = std::vector<FitParameter<double>>;
} // namespace ComPWA

#endif
