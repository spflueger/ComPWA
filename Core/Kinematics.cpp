//-------------------------------------------------------------------------------
// Copyright (c) 2013 Peter Weidenkaff.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//		Peter Weidenkaff -
//-------------------------------------------------------------------------------

#include <boost/log/trivial.hpp>
using namespace boost::log;

#include "Core/Kinematics.hpp"

Kinematics* Kinematics::instance(){
	if(!_inst) {
//		BOOST_LOG_TRIVAIL(fatal)<<"Kinematics::instance() no instance available. you have to create one first";
				exit(1);
	}

	return Kinematics::_inst;
}

Kinematics* Kinematics::_inst = 0;