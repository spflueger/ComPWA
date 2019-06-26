#ifndef CPP_CORE_VECTORIZEDEVALUATOR_HPP_
#define CPP_CORE_VECTORIZEDEVALUATOR_HPP_

#include "FunctionGraph.hpp"

namespace ComPWA {

// IDEA: I think its best if I leave the FunctionGraph simple, and just like a function, that can be evaluated
// If I want to do fitting I need an estimator, which would wrap/decorate this FunctionGraph and do all of the
// create pipelines, parameter changed -> calculate mask, determine which pipeline to run etc...
// So at this point I have the information about the parameters (if fixed or not). this estimator (decorator) gets the additional function
// setParameterFitSettings() which sets a parameter fixed or not, then the full parameter set is just reduced to the non fixed ones
// This means that this new estimator would also limit itself to our own functiongraph version. but i don't think that is bad....
//
// another thing is how is the whole graph going to be executed.
// for example: we can have a set of pipelines (a branch pipeline, which would be just a vector of connected operations)
// depending on the parameters which changed. then at each fit iteration, we just execute the pipeline that corresponds
// to that change and run that pipeline. the pipeline itself does not take much storage in memory, so having multiple of the does not matter
// then res should be just the vector of the results that we want to return from the interface
// IMPORTANT: the mask mentioned above is crucial. So we have a set of branch or partial pipelines, which make up the full graph.
// Every parameter maps to one of these branch pipelines, this is surjective. With a mask we can determine if a certain partial pipeline
// should be executed in the next evaluation in an efficient way.
template <typename OutputType>
class FunctionGraphEvaluator : public Function<OutputType>
{
public:
    // this will take a functiongraph and data, and connect everything in a fixed way
    FunctionGraphEvaluator(FunctionGraph<OutputType> g) : Graph(g)
    {
    }

    void performOptimizations()
    {
        // perform graph optimizations, such as
        // - dynamic caching of intermediate edges
        // - presize all intermediate edges accordingly
        // - reseat data containers if possible to reduce memory usage (without speed loss)
        // calculate the mapping between parameter and partial/branch pipelines
    }

    void createPipelines()
    {
        // few things that are important:
        // 1. we want to shadow graph branches (up to their leaves), which do not have any
        // non-fixed parameter leaves. Just add a cached node on top of that branch since this
        // value is always constant during the fit. This has to be optional though, since it might
        // take to much memory.
        // 2. when parameters become fixed or non-fixed, different parts of the graph
        // have to be precalculated and shadowed
        // 3. the nodes have to be in a hierarchy, so that you know which node has to be calculated
        // before another etc. This is easy though, since you have all of the connections (edges)
        // 4. so in the initialization phase, we create a map from changed parameter mask to pipeline
        // Then we simply have to execute that pipeline when we got a new parameter set
        // 5. some data might be used in multiple nodes, in that case it would make sense to cache
        // that cache that data automatically
        // 6. data which is cached represents the endpoint of one pipeline, and the entrypoint for
        // new branch pipelines.
        // 7. a branch pipeline calculates only a certain part of the full graph
        // (but all of the intermediate datas are temporary)
    }

    //void attachFunctionGraphToEdge(edgeid id, functiongraph g, datavectors data)
    //{
    // get nodes, edges from that graph and incorporate into this graph
    //}

    OutputType evaluate()
    {
        //Here we do not call the evaluate of the functiongraph
        // then just process the pipelines we created before (depending on which parameters changed)
        // after the pipeline call, we switch back to the default pipeline (assumes nothing changed? so just returns result)
    }

    void updateParametersFrom(const ParameterList &list)
    {
        // update all parameters
        // at the same time determine which parameters have changed
        // then select a new current pipeline, depending on the list of changed parameters
    }

    ParameterList getParameters() const
    {
        // just call the getParameters function from the functiongraph
    }

private:
    FunctionGraph<OutputType> Graph;
};

} /* namespace ComPWA */

#endif
