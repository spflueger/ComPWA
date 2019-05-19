#ifndef CPP_CORE_FUNCTIONGRAPH_HPP_
#define CPP_CORE_FUNCTIONGRAPH_HPP_

#include <vector>

#include "Interfaces/Intensity.hpp"

namespace ComPWA {

// a node is an operation using the data from the ingoing edges
// and producing data to the outgoing edges
// the operation is arbitrary
// that the in and outgoing edges match this operation type
// has to be checked on construction
template<typename T>
class FunctionGraphNode {
public:
	virtual T evaluate() = 0;

	unsigned int UniqueId;
};

template<typename T>
class FunctionGraphEdge {
	unsigned int UniqueId;
	std::unique_ptr<FunctionGraphNode<T>> Source;
	std::unique_ptr<FunctionGraphNode<T>> Destination;
};

// simply defines a graph
// has functionality to add nodes and edges
// has functionality to find nodes and edges
class FunctionGraph {
public:
	FunctionGraph();
	virtual ~FunctionGraph();

	void addNode();
	void addEdge();

private:
	std::vector<std::unique_ptr<FunctionGraphNode>> Nodes;
	std::vector<FunctionGraphEdge> Edges;
};

// each graph node creates the in and out edges it needs
// these edges have to be connected later on using the graph
// member functions
template<typename X, typename Y>
class SummationGraphNode: public FunctionGraphNode<T> {
public:
	SummationGraphNode() {
		// create IngoingEdges
		// create OutgoingEdges
	}
	T evaluate() final {
		std::transform(IngoingEdges.begin(), IngoingEdges.end(), dv->values().begin(),
				results.begin(), std::plus<std::complex<double>>());
	}

private:
	std::vector<FunctionGraphEdge> IngoingEdges;
	std::vector<FunctionGraphEdge<T>> OutgoingEdges;
};

// This Evaluator should inherit from the Function<T> interface?
// a estimator would be just a FunctionGraphEvaluator<double> since it
// returns just a single estimator value like the log likelihood
template<typename T>
class FunctionGraphEvaluator: public Function<T> {
public:
	// initializes the Evaluator.
	// maybe go through the graph and prepare pipeline that evaluates
	// the graph (flow of data through the graph)
	FunctionGraphEvaluator(const FunctionGraph &graph);

	// this function just evaluates the whole graph and returns the result(s)
	// so we have a list of nodes in a hierarchy order, that have data sinks and sources
	// so we retrieve the data for the lowest nodes first and pass them to the nodes
	// and evaluate the node
	// then we take the output of that node, and deliver the data to the next node
	// this node was connected to.
	T evaluate() {

	}

	// so on each updateParametersFrom step, the evaluator has to determine
	// from which nodes to start recalculation.
	// and calculate the graph from those points upwards
	virtual void updateParametersFrom(const ParameterList &list) = 0;

	virtual ParameterList getParameters() const = 0;

private:
	FunctionGraph Graph;
};

}

#endif
