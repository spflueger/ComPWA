// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_INTENSITY_HPP_
#define COMPWA_INTENSITY_HPP_

#include "Core/ParameterList.hpp"

namespace ComPWA {

struct DataPoint;

///
/// \class Intensity
/// Pure interface class, resembling a real valued function. It can be evaluated
/// using DataPoints, which are elements of the domain of the function. For
/// example it can resemble a transition probability from an initial state to a
/// final state. However note that an Intensity can be, but does not have to be
/// normalized.
///
template<typename T>
class Function {
public:
	virtual ~Function() = default;

	/// evaluate intensity of model at \p point in phase-space
	virtual std::vector<T> evaluate(
			const std::vector<DataPoint> &point) const = 0;

	virtual void updateParametersFrom(const ParameterList &list) = 0;

	virtual ParameterList getParameters() const = 0;
};

using Intensity = Function<double>;
using Amplitude = Function<std::complex<double>>;

}
#endif
