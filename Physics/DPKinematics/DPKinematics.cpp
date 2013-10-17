/*
 * DPKinematics.cpp
 *
 *  Created on: Oct 16, 2013
 *      Author: weidenka
 */

#include "Physics/DPKinematics/DPKinematics.hpp"
#include "Physics/DPKinematics/DPpoint.hpp"
#include "Physics/DPKinematics/dataPoint.hpp"

void DPKinematics::init(){
	m23_sq_min= ((m2+m3)*(m2+m3));
	m23_sq_max=((M-m1)*(M-m1));
	m13_sq_min=((m1+m3)*(m1+m3));
	m13_sq_max=((M-m2)*(M-m2));
	m12_sq_min=((m1+m2)*(m1+m2));
	m12_sq_max=((M-m3)*(M-m3));
	m23_min=((m2+m3)); m23_max=((M-m1));
	m13_min=((m1+m3)); m13_max=((M-m2));
	m12_min=((m1+m2)); m12_max=((M-m3));
};
DPKinematics::DPKinematics(double _M, double _Br, double _m1, double _m2, double _m3, std::string _name1, std::string _name2, std::string _name3):
				M(_M), Br(_Br), m1(_m1), m2(_m2), m3(_m3), name1(_name1), name2(_name2), name3(_name3)
{
	init();
};
DPKinematics::DPKinematics(const DPKinematics& other):
				M(other.M), Br(other.Br),
				m1(other.m1), m2(other.m2), m3(other.m3),
				name1(other.name1), name2(other.name2), name3(other.name3)
{
	init();
};
bool DPKinematics::isWithinDP() const{
	static dataPoint* point = dataPoint::instance();
	double s1 = point->getMsq(2,3);
	double s2 = point->getMsq(1,3);
	double s3 = point->getMsq(1,2);
	return isWithinDP(s1,s2,s3);
}
bool DPKinematics::isWithinDP(double m23, double m13, double m12) const{
	if(m13<s2max(m23) && m13>s2min(m23) &&m12<s3max(m23) && m12>s3min(m23) &&m23<s1max(m13) && m23>s1min(m13) ) return 1;
	return 0;
}

double DPKinematics::lambda(double x, double y, double z)const{
	return x*x+y*y+z*z-2.*x*y-2.*x*z-2.*y*z;
}

Double_t DPKinematics::s2min(Double_t s1, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	Double_t result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) - lamterm )/(2.*s1);

	return result;
}

Double_t DPKinematics::s2max(Double_t s1, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	Double_t result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) + lamterm )/(2.*s1);

	return result;
}

Double_t DPKinematics::s3min(Double_t s1, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	Double_t result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m2*m2) - lamterm )/(2.*s1);

	return result;
}

Double_t DPKinematics::s3max(Double_t s1, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	Double_t result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m3*m3) + lamterm )/(2.*s1);

	return result;
}

Double_t DPKinematics::s1min(Double_t s2, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m3*m3, m1*m1) );

	Double_t result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) - lamterm )/(2.*s2);

	return result;
}

Double_t DPKinematics::s1max(Double_t s2, Double_t m0, Double_t m1, Double_t m2, Double_t m3)const
{
	Double_t s      = m0*m0;
	Double_t lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m1*m1, m3*m3) );

	Double_t result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) + lamterm )/(2.*s2);

	return result;
}
