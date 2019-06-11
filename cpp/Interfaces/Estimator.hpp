// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_ESTIMATOR_ESTIMATOR_HPP_
#define COMPWA_ESTIMATOR_ESTIMATOR_HPP_

#include "Function.hpp"

namespace ComPWA {
namespace Estimator {

///
/// This class provides the interface to classes which estimate the "closeness"
/// of the modeled intensity to the data. Any derived Estimator can be used with
/// any derived Optimizer.
///
template <typename T> class Estimator : public Function<T, void> {

public:
  virtual ~Estimator() = default;

  /// Evaluates the Estimator, which calculates the "distance" of the
  /// Intensity from the DataPoints (or more generally a model from the data).
  /// The Optimizer tries to minimize/optimize the returned value of the
  /// Estimators evaluate function.
  virtual T evaluate() const = 0;
};

} // namespace Estimator
} // namespace ComPWA

#endif
