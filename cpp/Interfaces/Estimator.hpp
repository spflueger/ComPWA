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
/// Estimators are functions which are evaluated without data
template <typename OutputType>
class Estimator : public Function<OutputType>
{
public:
  virtual ~Estimator() = default;
};

// this is what a log likelihood estimator would look like
class MaxLogLHEstimator : public Estimator<double>
{
    double evaluate() final;
    // changes parameters to the given values in the list
    void updateParametersFrom(const ParameterList &) final;
    // gets a list of parameters defined by this function
    ParameterList getParameters() const final;
};

} // namespace Estimator
} // namespace ComPWA

#endif
