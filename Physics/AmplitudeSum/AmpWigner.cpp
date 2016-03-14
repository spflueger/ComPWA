//-------------------------------------------------------------------------------
// Copyright (c) 2013 Mathias Michel.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Mathias Michel - initial API and implementation
//     Peter Weidenkaff -  assignment of final state particle masses
//-------------------------------------------------------------------------------
//****************************************************************************
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.
//****************************************************************************

// --CLASS DESCRIPTION [MODEL] --
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.

#include <cmath>
#include "Physics/AmplitudeSum/AmpWigner.hpp"

#include "qft++.h"

namespace ComPWA {
namespace Physics {
namespace AmplitudeSum {

AmpWigner::AmpWigner()
{
	toEvaluate=false;
}
AmpWigner::AmpWigner(const char *name,
		unsigned int spin, unsigned int m,  unsigned int n,unsigned int subSys) :
												  _inSpin(spin),
												  _outSpin1(m),
												  _outSpin2(n),
												  _subSys(subSys)
{
	toEvaluate=true;
	initialise();
}

AmpWigner::AmpWigner(const AmpWigner& other, const char* newname) :
												  _inSpin(other._inSpin),
												  _outSpin1(other._outSpin1),
												  _outSpin2(other._outSpin2),
												  _subSys(other._subSys),
												  toEvaluate(other.toEvaluate)
{
	initialise();
}

AmpWigner::~AmpWigner() 
{
}

void AmpWigner::initialise() 
{
  DPKinematics::DalitzKinematics* kin = dynamic_cast<DPKinematics::DalitzKinematics*>(Kinematics::instance());
	_M=kin->M;
	//if(_subSys==5){
	_m1=kin->m1;
	_m2=kin->m2;
	_m3=kin->m3;//}
	//if(_subSys==4){
	//_m1=point->DPKin.m2;
	//_m2=point->DPKin.m3;
	//_m3=point->DPKin.m1;//}
	//if(_subSys==3){
	//_m1=point->DPKin.m3;
	//_m2=point->DPKin.m2;
	//_m3=point->DPKin.m1;//}
	//	cout<<"AmpWigner DEBUG set masses to m1="<<_m1<< " m2="<<_m2<<" m3=" <<_m3<<endl;

}    
void AmpWigner::setDecayMasses(double m1, double m2, double m3, double M){
	_M=M; _m1=m1; _m2=m2; _m3=m3;
	return;
}

double AmpWigner::evaluate(const dataPoint& point) const {
	if(!toEvaluate) return 1.;
	DPKinematics::DalitzKinematics* kin = dynamic_cast<DPKinematics::DalitzKinematics*>(Kinematics::instance());

	double locmin_sq, locmax_sq, beta;

/*
//	cout<<"==== "<<_subSys<< " " <<_m1<< " "<<_m2<<" " <<_m3<<endl;
	double invM1 = dataPoint::instance()->getM(_subSys);
	int mod=0;
	if(_subSys==5) mod=4; //5->3 work also without beta=nan, for agreement with Laura++ 5->4 is correct
	if(_subSys==4) mod=5;
	if(_subSys==3) mod=5;
	double invM2 = dataPoint::instance()->getM(mod);
	double altm23_sq = dataPoint::instance()->getM(5); altm23_sq*=altm23_sq;
	double altm13_sq = dataPoint::instance()->getM(4); altm13_sq*=altm13_sq;
	double altm12_sq = dataPoint::instance()->getM(3); altm12_sq*=altm12_sq;

//	dataPoint* point = dataPoint::instance();
	//	cout<<point->getM(3)<<" " <<point->getM(4)<< " " << point->getM(5)<<endl;
	//	double locmin_sq2 = s2min(_m23*_m23,_M,_m1,_m2,_m3);
	//	double locmax_sq2 = s2max(_m23*_m23,_M,_m1,_m2,_m3);
	//	double beta2=acos((2.*_m13*_m13-locmax_sq2-locmin_sq2)/(locmax_sq2-locmin_sq2));

	double altCosTheta=0;
	  switch(_subSys){
		case 5:{ //reso in m23
		  locmin_sq = s2min(invM1*invM1,_M,_m1,_m2,_m3);
		  locmax_sq = s2max(invM1*invM1,_M,_m1,_m2,_m3);
		  altCosTheta = cosTheta(altm23_sq,altm12_sq,altm13_sq);
		  break;
		}
		case 4:{ //reso in m13
		  locmin_sq = s1min(invM1*invM1,_M,_m1,_m2,_m3);
		  locmax_sq = s1max(invM1*invM1,_M,_m1,_m2,_m3);
		  altCosTheta = cosTheta(altm13_sq,altm12_sq,altm23_sq);
		  break;
		}
		case 3:{ //reso in m12
		  //return 1;
		  locmin_sq = s1min(invM1*invM1,_M,_m1,_m3,_m2);
		  locmax_sq = s1max(invM1*invM1,_M,_m1,_m3,_m2);
		  altCosTheta = cosTheta(altm12_sq,altm23_sq,altm13_sq);
		  //if(beta!=beta) return 1.;
		  break;
		}
	  }
	double cosbeta = (2.*invM2*invM2-locmax_sq-locmin_sq)/(locmax_sq-locmin_sq);
//	if( cosbeta > 1 && cosbeta < 1.1) cosbeta=1;
    beta=acos(cosbeta);
    //beta=acos(altCosTheta);
	//if(_subSys!=5) beta=acos(1);

	//	cout<<"==== "<<_m23<< " "<<_m13<< " "<<invM1<<" " <<invM2<<endl;
	//	cout<< "wwww " <<_subSys<< " "<<mod<<" " <<beta<< " "<<beta2<<endl;
	//	if(beta!=beta){
	//		std::cout<<beta<<std::endl;
	//		std::cout<<_M<< " "<<_m1<<" " <<_m2<<" " <<_m3<<std::endl;
	//		std::cout<<(2.*_m13*_m13-locmax_sq-locmin_sq)/(locmax_sq-locmin_sq)<<std::endl;
	//		std::cout<<_m13<< " " <<_m23<<" " <<locmin_sq << " "<<locmax_sq<<std::endl;
*/
//	double m23sq = point.getVal("m23sq");
//	double m13sq = point.getVal("m13sq");
	double m23sq = point.getVal(0);
	double m13sq = point.getVal(1);
	double m12sq = kin->getThirdVariableSq(m23sq,m13sq);
	double invM1 = -999;
	double invM2 = -999;
	switch(_subSys){
	case 3: {
		invM1=sqrt(m12sq);
		invM2=sqrt(m23sq);
		locmin_sq = kin->invMassMin(5,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(5,_subSys,invM1*invM1);
		break;
	}
	case 4: {
		invM1=sqrt(m13sq);
		invM2=sqrt(m12sq);
		locmin_sq = kin->invMassMin(5,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(5,_subSys,invM1*invM1);
		break;
	}
	case 5: {
		invM1=sqrt(m23sq);
		invM2=sqrt(m13sq);
		locmin_sq = kin->invMassMin(4,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(4,_subSys,invM1*invM1);
		break;
	}
	}

	//	switch(_subSys){
	//	case 5:{ //reso in m23
	//		locmin_sq = s2min(invM1*invM1,_M,_m1,_m2,_m3);
	//		locmax_sq = s2max(invM1*invM1,_M,_m1,_m2,_m3);
	//		break;
	//	}
	//	case 4:{ //reso in m13
	//		locmin_sq = s1min(invM1*invM1,_M,_m1,_m2,_m3);
	//		locmax_sq = s1max(invM1*invM1,_M,_m1,_m2,_m3);
	//		break;
	//	}
	//	case 3:{ //reso in m12
	//		//return 1;
	//		locmin_sq = s1min(invM1*invM1,_M,_m1,_m3,_m2);
	//		locmax_sq = s1max(invM1*invM1,_M,_m1,_m3,_m2);
	//		//if(beta!=beta) return 1.;
	//		break;

	//	}
	//	}
	double cosbeta = (2.*invM2*invM2-locmax_sq-locmin_sq)/(locmax_sq-locmin_sq);
	beta=acos(cosbeta);

	Spin j((double) _inSpin), m((double) _outSpin1), n((double)_outSpin2);
	double result = Wigner_d(j,m,n,beta);
	if( ( result!=result ) || (beta!=beta)) {
		std::cout<<"= AmpWigner: "<<std::endl;
		std::cout<< "NAN! J="<< _inSpin<<" M="<<_outSpin1<<" N="<<_outSpin2<<" beta="<<beta<<std::endl;
		std::cout<< "subSys: "<<_subSys<<" invM1sq="<<invM1*invM1 << " invM2sq=" <<invM2*invM2<< " cos(beta)="<<cosbeta<<std::endl;
		return 0;
	}
//	std::cout<<"===================================="<<std::endl;
	return result;
}

double AmpWigner::cosTheta(double s, double t, double u)const{
  return ( s*(t-u) + (_M*_M-_m1*_m1)*(_m2*_m2-_m3*_m3) )/( 4*s*qin(s)*qout(s) );
}

double AmpWigner::qin(double s)const{
  double square = (s-(_M+_m1)*(_M+_m1)) * (s-(_M-_m1)*(_M-_m1)) / (4*s);
  return sqrt(square);
}

double AmpWigner::qout(double s)const{
  double square = (s-(_m2+_m3)*(_m2+_m3)) * (s-(_m2-_m3)*(_m2-_m3)) / (4*s);
  return sqrt(square);
}

double AmpWigner::lambda(double x, double y, double z)const{
	return x*x+y*y+z*z-2.*x*y-2.*x*z-2.*y*z;
}

double AmpWigner::s2min(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	double result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) - lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s2max(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	double result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) + lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s3min(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	double result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m2*m2) - lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s3max(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	double result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m3*m3) + lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s1min(double s2, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m3*m3, m1*m1) );

	double result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) - lamterm )/(2.*s2);

	return result;
}

double AmpWigner::s1max(double s2, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m1*m1, m3*m3) );

	double result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) + lamterm )/(2.*s2);

	return result;
}

} /* namespace AmplitudeSum */
} /* namespace Physics */
} /* namespace ComPWA */
