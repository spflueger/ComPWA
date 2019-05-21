#include "FunctionGraph.hpp"
#include <algorithm>
#include <functional>
#include <memory>

namespace ComPWA {

// defines a standard binary operation in a vectorized way using std::vector
template <typename InputType1, typename InputType2, typename OutputType>
class VectorizedBinaryOperationFunctor {
public:
  VectorizedBinaryOperationFunctor(
      auto input1, auto input2, auto output,
      std::function<OutputType(InputType1, InputType2)> BinaryOperation)
      : Input1(input1), Input2(input2), Output(output),
        BinaryOperation(BinaryOperation) {}

  void operator()() {
    std::transform(Input1.begin(), Input1.end(), Input2.begin(),
                   std::back_inserter(Output), BinaryOperation);
  }

private:
  const std::vector<InputType1> &Input1;
  const std::vector<InputType1> &Input2;
  std::vector<OutputType> &Output;
  std::function<OutputType(InputType1, InputType2)> BinaryOperation;
};

template <typename BinaryOperation, typename InputType1, typename InputType2,
          typename OutputType>
std::vector<OutputType> binaryTransform(BinaryOperation function,
                                        const std::vector<InputType1> &Input1,
                                        const std::vector<InputType1> &Input2) {
  std::vector<OutputType> Output;
  std::transform(Input1.begin(), Input1.end(), Input2.begin(),
                 std::back_inserter(Output), function);
  return Output;
}

template <typename T> class DataSourceStrategy : public OperationStrategy {
public:
  DataSourceStrategy(const std::vector<T> &Data) : Data(Data) {}

  void operator()() final {}

private:
  const std::vector<T> &Data;
};

template <typename T>
class StrategyCachingDecorator : public OperationStrategy {
public:
  StrategyCachingDecorator() {}
  void operator()() final {
    if (false) {
      UndecoratedStrategy->operator()();
    }
  }

private:
  std::unique_ptr<OperationStrategy> UndecoratedStrategy;
  std::vector<T> CachedResult;
};

} /* namespace ComPWA */
