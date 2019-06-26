#ifndef CPP_CORE_FUNCTIONGRAPH_INTENSITYWRAPPER_HPP_
#define CPP_CORE_FUNCTIONGRAPH_INTENSITYWRAPPER_HPP_

#include "FunctionGraph.hpp"

namespace ComPWA {

class IntensityWrapper : public Intensity {
public:
  std::vector<double>
  evaluate(const std::vector<std::vector<double>> &DataPoints) {
    Graph.fillDataContainers(DataPoints);
    return Graph.evaluate().Values;
  }

  void updateParametersFrom(const ParameterList &list) {
    Graph.updateParametersFrom(list);
  }

  ParameterList getParameters() const { return Graph.getParameters(); }

private:
  FunctionGraph<Vector<double>> Graph;
};

} /* namespace ComPWA */

#endif
