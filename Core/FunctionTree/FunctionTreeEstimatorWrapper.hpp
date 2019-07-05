#ifndef CORE_FUNCTIONTREEESTIMATORWRAPPER_HPP_
#define CORE_FUNCTIONTREEESTIMATORWRAPPER_HPP_

#include <memory>

#include "Estimator/Estimator.hpp"
#include "ParameterList.hpp"

namespace ComPWA {
namespace FunctionTree {

class FunctionTree;

class FunctionTreeEstimatorWrapper
    : public ComPWA::Estimator::Estimator<double> {
public:
  FunctionTreeEstimatorWrapper(std::shared_ptr<FunctionTree> tree,
                               ParameterList parameters);
  double evaluate();
  void updateParametersFrom(const std::vector<double> &params);
  std::vector<double> getParameters() const;

private:
  std::shared_ptr<FunctionTree> Tree;
  ParameterList Parameters;
};

} // namespace FunctionTree
} /* namespace ComPWA */

#endif
