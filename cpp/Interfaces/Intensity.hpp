// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_INTENSITY_HPP_
#define COMPWA_INTENSITY_HPP_

#include <complex>
#include <string>

namespace ComPWA {

template <typename T> struct Value {
  std::string Name;
  unsigned int UniqueID;
  T Value;
};

enum class ErrorType { NOT_DEFINED, SYMMETRIC, ASYMMETRIC };

template <typename T> struct Parameter : public Value<T> {
  std::pair<T, T> Errors;
  std::pair<T, T> Bounds;
  bool HasBounds = false;
  bool IsFixed = false;
  ErrorType ErrType = ErrorType::NOT_DEFINED;
};

using ParameterList = std::vector<Parameter<double>>;

///
/// \class Function
/// Interface that resembles a function of the form
///
/// OutputType function(InputType)
///
/// The Function also defines a list of parameters, which can be altered
/// for example by Optimizers.
///
template <typename OutputType, typename InputType> class Function {
public:
  virtual ~Function() = default;

  /// evaluate intensity of model at \p point in phase-space
  virtual OutputType evaluate(const InputType &data) const = 0;

  // changes parameters to the given values in the list
  virtual void updateParametersFrom(const ParameterList &list) = 0;

  // gets a list of parameters defined by this function
  virtual ParameterList getParameters() const = 0;
};

using Intensity = Function<double, std::vector<double>>;
using Amplitude = Function<std::complex<double>, std::vector<double>>;

} // namespace ComPWA
#endif
