#ifndef CPP_CORE_FUNCTIONGRAPH_HPP_
#define CPP_CORE_FUNCTIONGRAPH_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "../Interfaces/Function.hpp"

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

enum class DataType { DOUBLE, COMPLEX_DOUBLE, INT };
enum class OperationType { ADD, MULTIPLY, SQRT, POW, EXP, LOG, COS };

// resembles an operation with a node
struct FunctionGraphNode {
  unsigned int UniqueId;
  OperationType OperationName;
};

// resembles a single node to node connection
struct FunctionGraphEdge {
  unsigned int UniqueId;
  DataType Type;
  unsigned int SourceNodeID;
  unsigned int SinkNodeID;
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
  // no dangling edges and unconnected nodes
  bool isCompletelyConnected() const;

  // these would be the top level nodes, which store the final result
  FunctionGraphNode getSinkNodes();

  void attachSubgraphToEdge(FunctionGraph SubGraph, unsigned int EdgeID);

private:
  std::vector<FunctionGraphNode> Nodes;
  std::vector<FunctionGraphEdge> Edges;
};



} // namespace ComPWA

#endif
