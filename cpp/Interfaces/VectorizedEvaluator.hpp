/*
 * VectorizedEvaluator.hpp
 *
 *  Created on: May 20, 2019
 *      Author: steve
 */

#ifndef CPP_CORE_VECTORIZEDEVALUATOR_HPP_
#define CPP_CORE_VECTORIZEDEVALUATOR_HPP_

#include "FunctionGraph.hpp"

namespace ComPWA {

// returns just a single estimator value like the log likelihood
class VectorizedEvaluator : public FunctionGraphEvaluator<double> {
public:
  // initializes the Evaluator.
  // maybe go through the graph and prepare pipeline that evaluates
  // the graph (flow of data through the graph)
  VectorizedEvaluator(const FunctionGraph &graph);

  // this function just evaluates the whole graph and returns the result(s)
  // so we have a list of nodes in a hierarchy order, that have data sinks and
  // sources so we retrieve the data for the lowest nodes first and pass them to
  // the nodes and evaluate the node then we take the output of that node, and
  // deliver the data to the next node this node was connected to.
  double evaluate(const std::vector<T> &data) const final;

  // so on each updateParametersFrom step, the evaluator has to determine
  // from which nodes to start recalculation.
  // and calculate the graph from those points upwards
  void updateParametersFrom(const ParameterList &list) final;

  ParameterList getParameters() const final;

private:
  FunctionGraph Graph;
};

} /* namespace ComPWA */

#endif /* CPP_CORE_VECTORIZEDEVALUATOR_HPP_ */
