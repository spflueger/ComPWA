// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_OPTIMIZER_HPP_
#define COMPWA_OPTIMIZER_HPP_

#include <string>
#include <map>

#include "Function.hpp"
#include "Estimator.hpp"

namespace ComPWA {

template <typename T> struct FitResult {
  ParameterList InitialParameters;
  ParameterList FinalParameters;

  double InitialEstimatorValue;
  double FinalEstimatorValue;

  double RuntimeInSeconds;

  T AdditionalProperties;
};

struct OptimizationSettings
{
    std::map<std::string, bool> FixedParameters;
};

///
/// \class Optimizer
/// This class provides the interface to (external) optimization libraries or
/// routines. As it is pure virtual, one needs at least one implementation to
/// provide an optimizer for the analysis which varies free model-parameters. If
/// a new optimizer is derived from and fulfills this base-class, no change in
/// other modules are necessary to work with the new optimizer library or
/// routine.
///
template <typename FitResultType, typename EstimatorType>
class Optimizer
{
    // Get the list of parame	ters from the estimator (via getParameters())
    // Compare with the Settings, and create a mapping of the fitted parameters
    // to their place in the complete list
    FitResult<FitResultType> optimize(const Estimator<EstimatorType> &Estimator, OptimizationSettings Settings) = 0;
};


} // namespace ComPWA

#endif
