// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_INTENSITY_HPP_
#define COMPWA_INTENSITY_HPP_

#include <complex>
#include <string>


namespace ComPWA {

struct Dimension
{
    size_t Size;
};

struct Tensor
{
    virtual ~Tensor() = default;
    std::vector<Dimension> Dimensions;
    // size: scalar = 0, vector = 1, matrix = 2
};

template <typename T>
struct Scalar : public Tensor
{
    Scalar(T value) : Value(value) {}
    T Value;
};

template <typename T>
struct Vector : public Tensor
{
    Vector() = default;
    Vector(std::vector<T> values) : Values(values)
    {
        Dimensions.push_back({Values.size()});
    }
    std::vector<T> Values;
};

// matrix
template <typename T>
struct Matrix : public Tensor
{
    std::vector<std::vector<T>> Values;
};

template <typename T>
struct Value
{
    std::string Name;
    T Value;
};

using ParameterList = std::vector<std::pair<unsigned int, Value<double>>>;

///
/// \class Function
/// Interface that resembles a function of the form
///
/// OutputType function(InputTypes)
///
/// The Function also defines a list of parameters, which can be altered
/// for example by Optimizers.
///
template <typename OutputType, typename... InputTypes>
class Function
{
public:
    virtual ~Function() = default;

    // we dont make the evaluate function const anymore, because
    // it will allow internal modification like caching
    virtual OutputType operator()(const ParameterList &list, const InputTypes &... args) = 0;

    // changes parameters to the given values in the list
    // The order of the parameters in the list is important.
    // It has to be the same as returned by getParameters()
   // virtual void updateParametersFrom(const ParameterList &) = 0;

    // gets a list of parameters defined by this function
    virtual ParameterList getParameters() const = 0;
};
using MDouble = std::vector<double>;
// and intensity is just a function which takes a list of data points and returns a list of intensities
using Intensity = Function<std::vector<double>, std::vector<MDouble>;

} // namespace ComPWA
#endif
