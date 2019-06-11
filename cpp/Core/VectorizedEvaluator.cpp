#include "VectorizedEvaluator.hpp"

#include <algorithm>
#include <functional>
#include <memory>

namespace ComPWA {

// defines a standard binary operation in a vectorized way using std::vector
template <typename BinaryOperator, typename InputType1, typename InputType2, typename OutputType>
class VectorizedBinaryOperationFunctor : public OperationStrategy {
public:
  VectorizedBinaryOperationFunctor(
		  BinaryOperator function, auto input1, auto input2, auto output)
      : Input1(input1), Input2(input2), Output(output),
        Function(function) {}

  void operator()() {
    std::transform(Input1.begin(), Input1.end(), Input2.begin(),
                   std::back_inserter(Output), Function());
  }

private:
  const std::vector<InputType1> &Input1;
  const std::vector<InputType2> &Input2;
  std::vector<OutputType> &Output;
  BinaryOperator Function;
};

template <typename Function, typename InputType1, typename InputType2,
          typename OutputType>
std::vector<OutputType>
vectorizedBinaryOperation(Function function,
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

  std::vector<T> getOutput();

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


template<class...Ts>
struct sink:std::function<void(Ts...)>{
  using std::function<void(Ts...)>::function;
};

template<class...Ts>
using source=sink<sink<Ts...>>;

template<class In, class Out>
using process=sink< source<In>, sink<Out> >;

template<class In, class Out>
sink<In> operator|( process< In, Out > a, sink< Out > b ){
  return [a,b]( In in ){
    a( [&in]( sink<In> s )mutable{ s(std::forward<In>(in)); }, b );
  };
}
template<class In, class Out>
source<Out> operator|( source< In > a, process< In, Out > b ){
  return [a,b]( sink<Out> out ){
    b( a, out );
  };
}

template<class In, class Mid, class Out>
process<In, Out> operator|( process<In, Mid> a, process<Mid, Out> b ){
  return [a,b]( source<In> in, sink<Out> out ){
    a( in, b|out ); // or b( in|a, out )
  };
}
template<class...Ts>
sink<> operator|( source<Ts...> a, sink<Ts...> b ){
  return[a,b]{ a(b); };
}


process<char, char> to_upper = []( source<char> in, sink<char> out ){
  in( [&out]( char c ) { out( std::toupper(c) ); } );
};

source<char> hello_world = [ptr="hello world"]( sink<char> s ){
  for (auto it = ptr; *it; ++it ){ s(*it); }
};
sink<char> print = [](char c){std::cout<<c;};

int main(){
  auto prog = hello_world|to_upper|print;
  prog();
}

void test() {
  std::vector<double> a;
  std::vector<double> b;

  auto output = vectorizedBinaryOperation<std::plus(), double, double, double>(
      std::plus(), a, b);
}

} // namespace ComPWA
