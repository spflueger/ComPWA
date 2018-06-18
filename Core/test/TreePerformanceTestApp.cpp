//-------------------------------------------------------------------------------
// Copyright (c) 2013 Mathias Michel.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Mathias Michel - initial API and implementation
//-------------------------------------------------------------------------------

///
/// \file
/// This application test uses the ComPWA FunctionTree to calculate a simple
/// equation with cached intermediate results. The Tree can be set up in two
/// different way's, one using the functionality of the FunctionTree and the
/// other setting up the nodes and links manually. The second method is shown
/// mainly for a better understanding on how the FunctionTree internally works.
///

#define BOOST_TEST_MODULE Core

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "Core/FitParameter.hpp"
#include "Core/FunctionTree.hpp"
#include "Core/Functions.hpp"
#include "Core/TreeNode.hpp"
#include "Core/Value.hpp"

#include "Physics/DecayDynamics/RelativisticBreitWigner.hpp"

using namespace ComPWA;

BOOST_AUTO_TEST_SUITE(FunctionTreePerformanceTest);

BOOST_AUTO_TEST_CASE(SimpleOperations) {

  //------------new Tree with multiDouble Par----------------
  size_t nElements = 100000;
  std::shared_ptr<FunctionTree> myTreeMultD;

  //------------SetUp the parameters for R = Sum of (a * b)-----------
  std::vector<double> nMasses, nPhsp;
  std::shared_ptr<Parameter> mParB(new FitParameter("parB", 2));
  std::shared_ptr<Parameter> mParD(new FitParameter("parD", 3));
  for (unsigned int i = 0; i < nElements; i++) {
    // std::shared_ptr<FitParameter> tmpA(new
    // FitParameter("parA_"+i,i+1));
    nMasses.push_back(i + 1);
    nPhsp.push_back(2 * i + 1);
    nPhsp.push_back(2 * i + 2);
  }
  auto mParA = std::make_shared<Value<std::vector<double>>>("parA", nMasses);
  auto mParC = std::make_shared<Value<std::vector<double>>>("parC", nPhsp);
  auto result = std::make_shared<Value<double>>();

  myTreeMultD = std::make_shared<FunctionTree>(
      "R", result, std::make_shared<AddAll>(ParType::DOUBLE));
  myTreeMultD->createNode("Rmass", MDouble("par_Rnass", nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "R");
  myTreeMultD->createNode("ab", MDouble("par_ab", nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "Rmass");
  myTreeMultD->createLeaf("a", mParA, "ab");
  myTreeMultD->createLeaf("b", mParB, "ab");
  myTreeMultD->createNode("Rphsp", std::make_shared<AddAll>(ParType::DOUBLE),
                          "Rmass"); // this node will not be cached
  myTreeMultD->createNode("cd", MDouble("par_cd", 2 * nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "Rphsp");
  myTreeMultD->createLeaf("c", mParC, "cd");
  myTreeMultD->createLeaf("d", mParD, "cd");

  //------------Trigger Calculation----------------
  auto start = std::chrono::system_clock::now();
  myTreeMultD->parameter();
  auto end = std::chrono::system_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  std::cout << "Calculation time: " << elapsed << " us\n";

  LOG(info) << "MultiDouble Setup and calculated R = Sum[a*b] with one leaf "
               "containing "
            << nElements << " elements";
  LOG(info) << std::endl << myTreeMultD;

  // now lets calculate this with a optimized version
  double b(2);
  double d(3);
  double resultvalue(0.0);
  double tempval(0.0);
  start = std::chrono::system_clock::now();
  for (unsigned int i = 0; i < nElements; ++i) {
    resultvalue += nMasses[i] * b;
    tempval += (nPhsp[2 * i] + nPhsp[2 * i + 1]);
  }
  resultvalue *= tempval * d;

  end = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();
  std::cout << "Calculation time (fast): " << elapsed << " us\n";

  BOOST_CHECK_EQUAL(result->value(), resultvalue);

  std::vector<double> temp1(nElements);
  std::vector<double> temp2(2 * nElements);
  resultvalue = 0.0;
  tempval = 0.0;
  start = std::chrono::system_clock::now();
  for (unsigned int i = 0; i < nElements; ++i) {
    temp1[i] = nMasses[i] * b;
  }
  for (unsigned int i = 0; i < nElements; ++i) {
    temp2[2 * i] = nPhsp[2 * i] * d;
    temp2[2 * i + 1] = nPhsp[2 * i + 1] * d;
  }
  for (unsigned int i = 0; i < temp2.size(); ++i) {
    tempval += temp2[i];
  }
  for (unsigned int i = 0; i < nElements; ++i) {
    temp2[i] = temp1[i] * tempval;
  }
  for (unsigned int i = 0; i < nElements; ++i) {
    resultvalue += temp2[i];
  }

  end = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();
  std::cout << "Calculation time (fast2): " << elapsed << " us\n";

  BOOST_CHECK_EQUAL(result->value(), resultvalue);
}

BOOST_AUTO_TEST_SUITE_END();




BOOST_AUTO_TEST_CASE(ComplexOperations) {

  //------------new Tree with multiDouble Par----------------
  size_t nElements = 100000;
  std::shared_ptr<FunctionTree> myTreeMultD;

  //------------SetUp the parameters for R = Sum of (a * b)-----------
  std::vector<double> nMasses, nPhsp;
  std::shared_ptr<Parameter> mParB(new FitParameter("parB", 2));
  std::shared_ptr<Parameter> mParD(new FitParameter("parD", 3));
  for (unsigned int i = 0; i < nElements; i++) {
    // std::shared_ptr<FitParameter> tmpA(new
    // FitParameter("parA_"+i,i+1));
    nMasses.push_back(i + 1);
    nPhsp.push_back(2 * i + 1);
    nPhsp.push_back(2 * i + 2);
  }
  auto mParA = std::make_shared<Value<std::vector<double>>>("parA", nMasses);
  auto mParC = std::make_shared<Value<std::vector<double>>>("parC", nPhsp);
  auto result = std::make_shared<Value<double>>();


  std::make_shared<Physics::DecayDynamics::BreitWignerStrategy>()

  	  auto tr = std::make_shared<FunctionTree>(
  	      "RelBreitWigner" + suffix, MComplex("", sampleSize),
  	      std::make_shared<BreitWignerStrategy>());

  	  tr->createLeaf("Mass", Mass, "RelBreitWigner" + suffix);
  	  tr->createLeaf("Width", Width, "RelBreitWigner" + suffix);
  	  tr->createLeaf("OrbitalAngularMomentum", (double)L,
  	                 "RelBreitWigner" + suffix);
  	  tr->createLeaf("MesonRadius", MesonRadius, "RelBreitWigner" + suffix);
  	  tr->createLeaf("FormFactorType", FormFactorType, "RelBreitWigner" + suffix);
  	  tr->createLeaf("MassA", DaughterMasses.first, "RelBreitWigner" + suffix);
  	  tr->createLeaf("MassB", DaughterMasses.second, "RelBreitWigner" + suffix);
  	  tr->createLeaf("Data_mSq[" + std::to_string(pos) + "]",
  	                 sample.mDoubleValue(pos), "RelBreitWigner" + suffix);


  myTreeMultD = std::make_shared<FunctionTree>(
      "R", result, std::make_shared<AddAll>(ParType::DOUBLE));
  myTreeMultD->createNode("Rmass", MDouble("par_Rnass", nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "R");
  myTreeMultD->createNode("ab", MDouble("par_ab", nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "Rmass");
  myTreeMultD->createLeaf("a", mParA, "ab");
  myTreeMultD->createLeaf("b", mParB, "ab");
  myTreeMultD->createNode("Rphsp", std::make_shared<AddAll>(ParType::DOUBLE),
                          "Rmass"); // this node will not be cached
  myTreeMultD->createNode("cd", MDouble("par_cd", 2 * nElements),
                          std::make_shared<MultAll>(ParType::MDOUBLE), "Rphsp");
  myTreeMultD->createLeaf("c", mParC, "cd");
  myTreeMultD->createLeaf("d", mParD, "cd");

  //------------Trigger Calculation----------------
  auto start = std::chrono::system_clock::now();
  myTreeMultD->parameter();
  auto end = std::chrono::system_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end -
start).count();
  std::cout<<"Calculation time: "<<elapsed<<" us\n";

  LOG(info) << "MultiDouble Setup and calculated R = Sum[a*b] with one leaf "
               "containing "
            << nElements << " elements";
  LOG(info) << std::endl << myTreeMultD;


  // now lets calculate this with a optimized version
  double b(2);
  double d(3);
  double resultvalue(0.0);
  double tempval(0.0);
  start = std::chrono::system_clock::now();
  for(unsigned int i = 0; i < nElements; ++i) {
        resultvalue += nMasses[i]*b;
        tempval += (nPhsp[2*i]+nPhsp[2*i+1]);
  }
  resultvalue *= tempval*d;

  end = std::chrono::system_clock::now();
  elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(end -
start).count();
  std::cout<<"Calculation time (fast): "<<elapsed<<" us\n";

  BOOST_CHECK_EQUAL(result->value(), resultvalue);
}

BOOST_AUTO_TEST_SUITE_END();*/
