/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include <cmath> 

//#include "Riostream.h"
//#include "TMath.h"

//#include "RooAbsReal.h"
#include "Core/Parameter.hpp"
//#include "RooAbsCategory.h"
//#include "RooLinkedListIter.h"

#include "qft++.h"

#include "Physics/AmplitudeSum/AmpSumOfAmplitudes.hpp"

//ClassImp(AmpSumOfAmplitudes);

AmpSumOfAmplitudes::AmpSumOfAmplitudes()
{

}

 AmpSumOfAmplitudes::AmpSumOfAmplitudes(const char *name)
 { 

 } 


 AmpSumOfAmplitudes::AmpSumOfAmplitudes(const AmpSumOfAmplitudes& other, const char* name)
 { 

 } 

 AmpSumOfAmplitudes::~AmpSumOfAmplitudes(){
   //something TODO?
 }

void AmpSumOfAmplitudes::addBW(std::shared_ptr<AmpAbsDynamicalFunction> theRes , std::shared_ptr<DoubleParameter> r, std::shared_ptr<DoubleParameter> phi, std::shared_ptr<AmpWigner> theAng) {
  _pdfList.push_back(theRes);
  _intList.push_back(r);
  _phaseList.push_back(phi);
  _angList.push_back(theAng);
}

void AmpSumOfAmplitudes::addBW(std::shared_ptr<AmpAbsDynamicalFunction> theRes , std::shared_ptr<DoubleParameter> r, std::shared_ptr<DoubleParameter> phi) {
  _pdfList.push_back(theRes);
  _intList.push_back(r);
  _phaseList.push_back(phi);
  _angList.push_back(std::shared_ptr<AmpWigner>(new AmpWigner()));
}


 double AmpSumOfAmplitudes::evaluate() const
 { 
//   RooComplex res;
   complex<double> res;
   //std::cout << "res = \t" << res.abs2() << std::endl;

   for(unsigned int i=0; i<_pdfList.size(); i++){
     double a = _intList[i]->GetValue();
     double phi = _phaseList[i]->GetValue();
     std::complex<double> eiphi(a*cos(phi),a*sin(phi));

     std::complex<double> twoJplusOne(2*_pdfList[i]->getSpin()+1);
     res = res + twoJplusOne * _pdfList[i]->evaluate() * eiphi * _angList[i]->evaluate();

   }

   return ( std::abs(res)*std::abs(res) );
 } 

 double AmpSumOfAmplitudes::evaluateSlice(std::complex<double>* reso, unsigned int nResos, unsigned int subSys=1) const
 { 
   // ENTER EXPRESSION IN TERMS OF VARIABLE ARGUMENTS HERE 
   std::complex<double> res;

   int itReso=0, sys=0;

   for(unsigned int i=0; i<_pdfList.size(); i++){
     double a = _intList[i]->GetValue();
     double phi = _phaseList[i]->GetValue();
     std::complex<double> eiphi (cos(phi), sin(phi));
     if(itReso<3) sys = 0; //TODO: way better!!!
     else if(itReso<5) sys = 1;
     //else sys = 2;
     //sys = itReso;

     if(_pdfList[i]->isSubSys(subSys))
       res = res + reso[sys] * _angList[i]->evaluate();
     else
       res = res + _pdfList[i]->evaluate() * a * eiphi * _angList[i]->evaluate();

     itReso++;
   }

   return (std::abs(res)*std::abs(res) );
 } 

 /*double AmpSumOfAmplitudes::evaluatePhi() const
 { 
   // ENTER EXPRESSION IN TERMS OF VARIABLE ARGUMENTS HERE 
   RooComplex res;

   AmpRelBreitWignerRes *pdf;
   DoubleParameter *theInt;
   DoubleParameter *thePhase;
   AmpWigner *ang;

   _pdfIter->Reset();
   _intIter->Reset();
   _phaseIter->Reset();
   _angIter->Reset();

   //   TIterator* _pdfIter = _pdfList.createIterator() ;
   //   AmpRelBreitWignerRes *pdf;


   while((pdf      = (AmpRelBreitWignerRes*)_pdfIter->Next()) &&
	 (theInt   = (DoubleParameter*)_intIter->Next())        &&
	 (thePhase = (DoubleParameter*)_phaseIter->Next())      &&
         (ang      = (AmpWigner*)_angIter->Next())  ) {
     double a = theInt->GetValue();
     double phi = thePhase->GetValue();
     RooComplex eiphi (cos(phi), sin(phi));

     res = res + pdf->evaluate() * a * eiphi * ang->evaluate();
   }

   return atan2(res.im(),res.re()); 
 } */

