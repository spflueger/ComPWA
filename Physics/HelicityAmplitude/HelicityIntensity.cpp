//-------------------------------------------------------------------------------
// Copyright (c) 2013 Stefan Pflueger.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//   Stefan Pflueger - initial API and implementation
//--------------------------------------------------------------------------------

#include "HelicityIntensity.hpp"

namespace HelicityFormalism {

HelicityIntensity::HelicityIntensity() {
	// TODO Auto-generated constructor stub

}

HelicityIntensity::~HelicityIntensity() {
	// TODO Auto-generated destructor stub
}

const ParameterList& HelicityIntensity::intensity(std::vector<double> point,
		ParameterList& par) {
	setParameterList(par);
	dataPoint dataP(point);
	return intensity(dataP);
}

const ParameterList& HelicityIntensity::intensity(dataPoint& point,
		ParameterList& par) {
	setParameterList(par);
	return intensity(point);
}

const ParameterList& HelicityIntensity::intensityNoEff(dataPoint& point) {
	std::complex<double> intensity = 0;
	if (Kinematics::instance()->isWithinPhsp(point)) {
		for (unsigned int i = 0; i < amplitudes.size(); ++i) {
			intensity += amplitudes[i].evaluate(point);
		}
		intensity = pow(std::abs(intensity), 2.0);
	}

	if (intensity != intensity) {
		BOOST_LOG_TRIVIAL(error)<<"Intensity is not a number!!";
		intensity = 0;
	}
	result.SetParameterValue(0, intensity);
	return result;
}

const ParameterList& HelicityIntensity::intensity(dataPoint& point) {
	intensityNoEff(point);
	double eff = efficiency->evaluate(point);
	result.SetParameterValue(0, result.GetDoubleParameter(0)->GetValue() * eff);
	return result;
}

const bool HelicityIntensity::fillStartParVec(ParameterList& outPar){
	outPar = ParameterList(params);
	return true;
}

void HelicityIntensity::setParameterList(ParameterList& par){
	//parameters varied by Minimization algorithm
	if(par.GetNDouble()!=params.GetNDouble())
		throw std::runtime_error("setParameterList(): size of parameter lists don't match");
	for(unsigned int i=0; i<params.GetNDouble(); i++){
		std::shared_ptr<DoubleParameter> p = params.GetDoubleParameter(i);
		if(!p->IsFixed()){
			p->SetValue(par.GetDoubleParameter(i)->GetValue());
			p->SetError(par.GetDoubleParameter(i)->GetError());
		}
	}
	return;
}

} /* namespace HelicityFormalism */
