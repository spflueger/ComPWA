#ifndef CPP_CORE_FUNCTIONGRAPH_HPP_
#define CPP_CORE_FUNCTIONGRAPH_HPP_

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <map>
#include <memory>

#include "Interfaces/Function.hpp"

#include <pstl/algorithm>
#include <pstl/execution>

namespace ComPWA {

// ------------------------ Now we define your ComPWA function graph backend! -------------------------

class OperationStrategy
{
public:
    virtual ~OperationStrategy() = default;

    virtual void execute() = 0;
};

// TODO: use result type traits to predefine the output type given the input types!

// defines a standard binary operation in a vectorized way using std::vector
template <typename OutputType, typename InputType1, typename InputType2, typename BinaryOperator>
class BinaryOperationFunctor : public OperationStrategy
{
public:
    BinaryOperationFunctor(
        std::shared_ptr<OutputType> output, std::shared_ptr<const InputType1> input1, std::shared_ptr<const InputType2> input2, BinaryOperator function)
        : Input1(input1), Input2(input2), Output(output),
          Function(function) {}

    void execute() final
    {
        Function(*Output, *Input1, *Input2);
    }

private:
    std::shared_ptr<const InputType1> Input1;
    std::shared_ptr<const InputType2> Input2;
    std::shared_ptr<OutputType> Output;
    BinaryOperator Function;
};

template <typename BinaryFunction>
struct ElementWiseBinaryOperation
{
    ElementWiseBinaryOperation(BinaryFunction f) : Function(f){};

    template <typename OutputType, typename InputType1, typename InputType2>
    void operator()(Scalar<OutputType> &Output, const Scalar<InputType1> &Input1, const Scalar<InputType2> &Input2)
    {
        Output.Value = Function(Input1.Value, Input2.Value);
    }

    template <typename OutputType, typename InputType1, typename InputType2>
    void operator()(Vector<OutputType> &Output, const Vector<InputType1> &Input1, const Vector<InputType2> &Input2)
    {
        std::transform(pstl::execution::par_unseq, Input1.Values.begin(), Input1.Values.end(), Input2.Values.begin(),
                       Output.Values.begin(), Function);
    }

    template <typename OutputType, typename InputType1, typename InputType2>
    void operator()(Vector<OutputType> &Output, const Vector<InputType1> &Input1, const Scalar<InputType2> &Input2)
    {
        std::transform(pstl::execution::par_unseq, Input1.Values.begin(), Input1.Values.end(),
                       Output.Values.begin(), [&Input2, this](const InputType1 &x) { return Function(x, Input2.Value); });
    }

private:
    BinaryFunction Function;
};

// defines a standard unary operation in a vectorized way using std::vector
template <typename OutputType, typename InputType, typename UnaryOperator>
class UnaryOperationFunctor : public OperationStrategy
{
public:
    UnaryOperationFunctor(
        std::shared_ptr<OutputType> output, std::shared_ptr<const InputType> input, UnaryOperator function)
        : Input(input), Output(output),
          Function(function) {}

    void execute() final
    {
        Function(*Output, *Input);
    }

private:
    std::shared_ptr<const InputType> Input;
    std::shared_ptr<OutputType> Output;
    UnaryOperator Function;
};

template <typename UnaryFunction>
struct ElementWiseUnaryOperation
{
    ElementWiseUnaryOperation(UnaryFunction f) : Function(f){};

    template <typename OutputType, typename InputType>
    void operator()(Scalar<OutputType> &Output, const Scalar<InputType> &Input)
    {
        Output.Value = Function(Input.Value);
    }

    template <typename OutputType, typename InputType>
    void operator()(Vector<OutputType> &Output, const Vector<InputType> &Input)
    {
        std::transform(pstl::execution::par_unseq, Input.Values.begin(), Input.Values.end(),
                       Output.Values.begin(), Function);
    }

private:
    UnaryFunction Function;
};

using EdgeID = size_t;
using NodeID = size_t;
using DataID = size_t;

enum struct EdgeType
{
    DATA = 0,
    PARAMETER = 1,
    TEMPORARY = 2
};

struct FunctionGraphEdge
{
    EdgeID UniqueID;
    NodeID Source;
    NodeID Sink;
    EdgeType Type;
};

struct FunctionGraphNode
{
    NodeID UniqueID;
    std::unique_ptr<OperationStrategy> Operation;
};

// Represents a function, via a graph structure
// - nodes are operations
// - edges are data
// Note: edges can be attached to 1 or 2 nodes
// It implements the Function interface, and is one of the possible calculation
// backends.
// The nodes and edges show the hierarchy of the operations. The evaluate
// function calls the standard pipeline, which returns the appropriate result.
// This means no caching of intermediate results is performed, since it does
// not make sense for a function
// Note that it is assumed on evaluation that all inputs are already connected to the graph
template <typename OutputType>
class FunctionGraph : public Function<OutputType>
{
public:
    FunctionGraph() = default;
    virtual ~FunctionGraph() = default;

    template <typename Output, typename Input1, typename Input2, typename FunctionType = void(Output &, const Input1 &, const Input2 &)>
    EdgeID
    addBinaryNode(FunctionType Function, EdgeID InputID1, EdgeID InputID2)
    {
        EdgeID OutEdgeID = createIntermediateEdge<Output>({InputID1, InputID2});
        //std::cout << "binary size of output container: " << std::any_cast<const Output &>(getDataReference(OutEdgeID)).size() << std::endl;
        createNewNode(std::make_unique<BinaryOperationFunctor<Output, Input1, Input2, FunctionType>>(
            getDataReference<Output>(OutEdgeID),
            getDataReference<const Input1>(InputID1),
            getDataReference<const Input2>(InputID2),
            Function));
        return OutEdgeID;
    }

    template <typename Output, typename Input, typename FunctionType = void(Output &, const Input &)>
    EdgeID
    addUnaryNode(FunctionType Function, EdgeID InputID, bool CacheNode = false)
    {
        EdgeID OutEdgeID = createIntermediateEdge<Output>({InputID});
        //std::cout << "size of input container: " << std::any_cast<const Input &>(getDataReference(InputID)).size() << std::endl;
        //std::cout << "size of output container: " << std::any_cast<const Output &>(getDataReference(OutEdgeID)).size() << std::endl;
        auto a = getDataReference<Output>(OutEdgeID);
        createNewNode(std::make_unique<UnaryOperationFunctor<Output, Input, FunctionType>>(
            a,
            getDataReference<const Input>(InputID),
            Function));
        return OutEdgeID;
    }

    template <typename DataType>
    EdgeID createDataSource(DataType Data)
    {
        FunctionGraphEdge NewEdge;
        auto edgeid = getNewEdgeID();
        NewEdge.UniqueID = edgeid;
        NewEdge.Type = EdgeType::DATA;
        Edges.push_back(NewEdge);
        auto dataid = DataStorage.size();
        EdgeToDataMapping[edgeid] = dataid;
        DataStorage[dataid] = std::make_shared<DataType>(Data);
        return edgeid;
    }

    template <typename ParameterType>
    EdgeID createParameterEdge(ParameterType data)
    {
        FunctionGraphEdge NewEdge;
        auto edgeid = getNewEdgeID();
        NewEdge.UniqueID = edgeid;
        NewEdge.Type = EdgeType::PARAMETER;
        Edges.push_back(NewEdge);
        auto dataid = DataStorage.size();
        EdgeToDataMapping[edgeid] = dataid;
        DataStorage[dataid] = std::make_shared<ParameterType>(data);
        return edgeid;
    }

    virtual OutputType evaluate()
    {
        for (const auto &node : Nodes)
        {
            node.Operation->execute();
        }
        std::cout << "finished calc. returning data\n";
        auto b = *getDataReference<OutputType>(TopEdge);
        std::cout << "result size: " << b.Values.size() << std::endl;
        return b;
    }

    void updateParametersFrom(const ParameterList &list)
    {
        // the argument parameterlist does not have to contain the full set of parameters or?
        // because only a subset of parameters might be free. that means we would need a unique
        // way to identify a parameter. (maybe give it a unique id?)
    }

    ParameterList getParameters() const
    {
    }

    void fillDataContainers(const std::vector<std::vector<double>> &data)
    {
        //loop over the data containers, and fill them the data given here

        //(this procedure might also reshape them)
        // only call resizeDataContainers,
        // on branch parts where the data containers do not match in size
        presizeDataContainers();
    }

private:
    template <typename T>
    std::shared_ptr<T> getDataReference(EdgeID edgeid)
    {
        std::cout << "edgeid: " << edgeid << "\n";
        std::cout << "size mapping: " << EdgeToDataMapping.size() << "\n";
        auto a = EdgeToDataMapping.at(edgeid);
        std::cout << "dataid: " << a << "\n";
        std::cout << "datastorage size: " << DataStorage.size() << std::endl;
        return std::dynamic_pointer_cast<T>(DataStorage.at(a));
    }

    void presizeDataContainers()
    {
        // basically we loop over the data containers
        // and if they are TEMPORARY and of dimension > 0
        // we resize them according to the input
        // if the input is not set, then we do the same for that data container....
        // (recursive??)
        //if DataStorage.at(id) ==)
    }

    template <typename T>
    EdgeID createIntermediateEdge(std::vector<EdgeID> InputEdgeIDs)
    {
        FunctionGraphEdge NewEdge;
        auto edgeid = getNewEdgeID();
        NewEdge.UniqueID = edgeid;
        NewEdge.Type = EdgeType::TEMPORARY;
        Edges.push_back(NewEdge);
        DataID dataid;
        bool FoundAvailableEdge(false);
        for (auto x : InputEdgeIDs)
        {
            /* auto result = std::find_if(Edges.begin(), Edges.end(), [&x](auto const &e) { return e.UniqueID == x; });
            if (result != Edges.end())
            {
                if (result->Type != EdgeType::TEMPORARY)
                {
                    // skip if not temporary data
                    continue;
                }
            }*/
            try
            {
                auto tempdata = getDataReference<T>(x);
            }
            catch (const std::bad_cast &e)
            {
                // if this is not the correct type of container, then just keep looking
                continue;
            }

            dataid = EdgeToDataMapping.at(x);
            FoundAvailableEdge = true;
            std::cout << "found good edge at " << dataid << "\n";
            break;
        }
        // if no suitable data container was found, create a new one
        if (!FoundAvailableEdge)
        {
            dataid = DataStorage.size();
            DataStorage[dataid] = std::make_shared<T>();
        }
        EdgeToDataMapping[edgeid] = dataid;

        TopEdge = edgeid;
        return edgeid;
    }

    EdgeID getNewEdgeID() const
    {
        return Edges.size();
    }

    NodeID createNewNode(std::unique_ptr<OperationStrategy> Op)
    {
        FunctionGraphNode NewNode;
        NewNode.UniqueID = getNewNodeID();
        NewNode.Operation = std::move(Op);
        Nodes.push_back(std::move(NewNode));
        return NewNode.UniqueID;
    }

    NodeID getNewNodeID() const
    {
        return Nodes.size();
    }

    std::vector<FunctionGraphNode> Nodes;
    std::vector<FunctionGraphEdge> Edges;
    // to reseat data elements, we need shared ptrs
    std::map<DataID, std::shared_ptr<Tensor>> DataStorage;
    std::map<EdgeID, DataID> EdgeToDataMapping;
    EdgeID TopEdge;
};



} // namespace ComPWA

#endif
