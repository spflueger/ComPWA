#ifndef CPP_CORE_FUNCTIONGRAPH_HPP_
#define CPP_CORE_FUNCTIONGRAPH_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "Intensity.hpp"

namespace ComPWA {

/*template <typename T> struct DataCommunicator {
  virtual ~DataCommunicator() = default;
  typename T getType() { return typeid(T); }
  virtual T getData() const = 0;
  virtual void sendData(T) = 0;
};*/

// this should be something like a function of arbitrary type without using
// templates....

/*class OperationStrategy {
public:
  virtual ~OperationStrategy() = default;

  virtual void operator()() const = 0;

  virtual void addSource(const DataCommunicator &Input) = 0;
  virtual void setSink(const DataCommunicator &Output) = 0;
};*/

// a node is an operation using the data from the ingoing edges
// and producing data to the outgoing edges
// the operation is arbitrary
// that the in and outgoing edges match this operation type
// has to be checked on construction

struct FunctionGraphNode {
  unsigned int UniqueId;

  std::function function;
};

struct FunctionGraphEdge {
  unsigned int UniqueId;

  FunctionGraphNode &Source;
  FunctionGraphNode &Sink;
};

// represents the structure of an arbitrary mathematical operation split up into
// its parts in other words, which data is processed by which operation etc how
// the data flows through the graph is handled by the evaluator classes which
// specify the actual pipeline for the calculation has functionality to add
// nodes and edges has functionality to find nodes and edges
class FunctionGraph {
public:
  FunctionGraph() = default;
  virtual ~FunctionGraph() = default;

  unsigned int addNode(std::function Operator);
  unsigned int connectNodes(unsigned int NodeIDIngoing,
                            unsigned int NodeIDOutgoing);

  // verify if the graph is completely connected and there are
  // no dangling edges and unconnnected nodes
  bool isCompletelyConnected() const;

  // these would be the top level nodes, which store the final result
  FunctionGraphNode getSinkNodes();

  void attachSubgraphToEdge(FunctionGraph SubGraph, unsigned int EdgeID);

private:
  std::vector<FunctionGraphNode> Nodes;
  std::vector<FunctionGraphEdge> Edges;
};

// This Evaluator should inherit from the Function<T> interface?
// a estimator would be just a FunctionGraphEvaluator<double> since it
// returns just a single estimator value like the log likelihood
template <typename Input, typename Output>
class FunctionGraphEvaluator : public Function<Input, Output> {
public:
  // initializes the Evaluator.
  // maybe go through the graph and prepare pipeline that evaluates
  // the graph (flow of data through the graph)
  FunctionGraphEvaluator(const FunctionGraph &graph);

  // this function just evaluates the whole graph and returns the result(s)
  // so we have a list of nodes in a hierarchy order, that have data sinks and
  // sources so we retrieve the data for the lowest nodes first and pass them to
  // the nodes and evaluate the node then we take the output of that node, and
  // deliver the data to the next node this node was connected to.
  virtual Output evaluate(const Input &data) const = 0;

  // so on each updateParametersFrom step, the evaluator has to determine
  // from which nodes to start recalculation.
  // and calculate the graph from those points upwards
  virtual void updateParametersFrom(const ParameterList &list) = 0;

  virtual ParameterList getParameters() const = 0;

private:
  FunctionGraph Graph;
};

} // namespace ComPWA

#endif
