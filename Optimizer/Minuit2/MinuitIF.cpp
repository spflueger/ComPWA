// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <boost/archive/xml_iarchive.hpp>

#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MinosError.h"
#include "Minuit2/MnHesse.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnStrategy.h"
#include "Minuit2/MnUserParameters.h"

#include "Core/Exceptions.hpp"
#include "Core/FitParameter.hpp"
#include "Core/Logging.hpp"
#include "Optimizer/Minuit2/MinuitIF.hpp"

using namespace ComPWA::Optimizer::Minuit2;

using namespace ROOT::Minuit2;

double shiftAngle(double v) {
  double originalVal = v;
  double val = originalVal;
  while (val > M_PI)
    val -= 2 * M_PI;
  while (val < (-1) * M_PI)
    val += 2 * M_PI;
  if (val != originalVal)
    LOG(INFO) << "shiftAngle() | Shifting parameter from " << originalVal
              << " to " << val << "!";
  return val;
}

MinuitIF::MinuitIF(OptimizationSettings Settings_) : Settings(Settings_) {}

MinuitResult MinuitIF::optimize(ComPWA::Estimator::Estimator<double> &Estimator,
                                ComPWA::FitParameterList InitialParameters) {
  LOG(DEBUG) << "MinuitIF::optimize() | Start";
  std::chrono::steady_clock::time_point StartTime =
      std::chrono::steady_clock::now();

  double InitialEstimatorValue(Estimator.evaluate());

  MnUserParameters upar;
  size_t FreeParameters(0);
  for (auto Param : InitialParameters) {
    if (Param.Name == "")
      throw BadParameter("MinuitIF::optimize() | FitParameter without name in "
                         "list. Since FitParameter names are unique we stop "
                         "here.");
    // If no error is set or error set to 0 we use a default error,
    // otherwise minuit treads this parameter as fixed
    double Error(0.001);
    if (Param.Value == 0.0)
      Error = Param.Value * 0.1;

    // Shift phase parameters to the range [-Pi,Pi]
    /* if (!Param.IsFixed &&
         actPat->name().find("phase") != actPat->name().npos)
       actPat->setValue(shiftAngle(actPat->value()));*/

    if (Param.HasBounds) {
      bool rt = upar.Add(Param.Name, Param.Value, Error, Param.Bounds.first,
                         Param.Bounds.second);
      if (!rt)
        throw BadParameter(
            "MinuitIF::optimize() | FitParameter " + Param.Name +
            " can not be added to Minuit2 (internal) parameters.");
    } else {
      bool rt = upar.Add(Param.Name, Param.Value, Error);
      if (!rt)
        throw BadParameter(
            "MinuitIF::optimize() | FitParameter " + Param.Name +
            " can not be added to Minuit2 (internal) parameters.");
    }
    if (!Param.IsFixed)
      FreeParameters++;
    if (Param.IsFixed)
      upar.Fix(Param.Name);
  }

  ROOT::Minuit2::MinuitFcn Function(Estimator);

  LOG(INFO) << "MinuitIF::optimize() | Number of parameters (free): "
            << InitialParameters.size() << " (" << FreeParameters << ")";

  // use MnStrategy class to set all options for the fit
  MinuitStrategy strat(1); // using default strategy = 1 (medium)

  try { // try to read xml file for minuit setting
    std::ifstream ifs("MinuitStrategy.xml");
    boost::archive::xml_iarchive ia(ifs, boost::archive::no_header);
    ia >> BOOST_SERIALIZATION_NVP(strat);
    strat.init(); // update parameters of MnStrategy mother class (IMPORTANT!)
    ifs.close();
    LOG(DEBUG) << "Minuit strategy parameters: from MinuitStrategy.xml";
  } catch (std::exception &ex) {
  }

  LOG(DEBUG) << "Gradient number of steps: " << strat.GradientNCycles();
  LOG(DEBUG) << "Gradient step tolerance: " << strat.GradientStepTolerance();
  LOG(DEBUG) << "Gradient tolerance: " << strat.GradientTolerance();
  LOG(DEBUG) << "Hesse number of steps: " << strat.HessianNCycles();
  LOG(DEBUG) << "Hesse gradient number of steps: "
             << strat.HessianGradientNCycles();
  LOG(DEBUG) << "Hesse step tolerance: " << strat.HessianStepTolerance();
  LOG(DEBUG) << "Hesse G2 tolerance: " << strat.HessianG2Tolerance();

  // MIGRAD
  MnMigrad migrad(Function, upar, strat);
  double maxfcn = 0.0;
  double tolerance = 0.1;

  // From the MINUIT2 Documentation:
  // Minimize the function MnMigrad()(maxfcn, tolerance)
  // \param maxfcn : max number of function calls (if = 0) default is
  //           used which is set to 200 + 100 * npar + 5 * npar**2
  // \param tolerance : value used for terminating iteration procedure.
  //           For example, MIGRAD will stop iterating when edm (expected
  //           distance from minimum) will be: edm < tolerance * 10**-3
  //           Default value of tolerance used is 0.1
  LOG(INFO) << "MinuitIF::exec() | Starting migrad: "
               "maxCalls="
            << maxfcn << " tolerance=" << tolerance;

  FunctionMinimum minMin = migrad(maxfcn, tolerance); //(maxfcn,tolerance)

  LOG(INFO) << "MinuitIF::exec() | Migrad finished! "
               "Minimum is valid = "
            << minMin.IsValid();

  // HESSE
  MnHesse hesse(strat);
  if (minMin.IsValid() && Settings.UseHesse) {
    LOG(INFO) << "MinuitIF::exec() | Starting hesse";
    hesse(Function, minMin); // function minimum minMin is updated by hesse
    LOG(INFO) << "MinuitIF::exec() | Hesse finished";
  } else
    LOG(INFO) << "MinuitIF::exec() | Migrad failed to "
                 "find minimum! Skip hesse and minos!";

  LOG(INFO) << "MinuitIF::exec() | Minimization finished! "
               "LH = "
            << std::setprecision(10) << minMin.Fval();

  /*
  // MINOS
  MnMinos minos(Function, minMin, strat);

  FitParameterList FinalParameters(InitialParameters);

  size_t id = 0;
  for (auto finalPar : FinalParameters) {
    if (finalPar.IsFixed)
      continue;

     if (finalPar->errorType() == ErrorType::ASYM) {
       // Skip minos and fill symmetic errors
       if (!minMin.IsValid() || !UseMinos) {
         LOG(INFO) << "MinuitIF::exec() | Skip Minos "
                      "for parameter "
                   << finalPar->name() << "...";
         finalPar->setError(minState.Error(finalPar->name()));
         continue;
       }
       // asymmetric errors -> run minos
       LOG(INFO) << "MinuitIF::exec() | Run minos "
                    "for parameter ["
                 << id << "] " << finalPar->name() << "...";
       MinosError err = minos.Minos(id);
       // lower = pair.first, upper= pair.second
       std::pair<double, double> assymErrors = err();
       finalPar->setError(assymErrors.first, assymErrors.second);
     } else if (finalPar->errorType() == ErrorType::SYM) {
       // symmetric errors -> migrad/hesse error
       finalPar->setError(minState.Error(finalPar->name()));
     } else {
       throw std::runtime_error(
           "MinuitIF::exec() | Unknown error type of parameter: " +
           std::to_string((long long int)finalPar->errorType()));
     }
     id++;
  }*/

  std::chrono::steady_clock::time_point EndTime =
      std::chrono::steady_clock::now();

  // Create fit result
  MinuitResult result = createResult(minMin, InitialParameters);

  result.InitialEstimatorValue = InitialEstimatorValue;
  result.FitDuration =
      std::chrono::duration_cast<std::chrono::seconds>(EndTime - StartTime);

  return result;
}

MinuitResult MinuitIF::createResult(ROOT::Minuit2::FunctionMinimum min,
                                    FitParameterList InitialParameters) {
  MinuitResult Result;

  MnUserParameterState minState = min.UserState();
  auto NumFreeParameter = minState.Parameters().Trafo().VariableParameters();

  // ParameterList can be changed by minos. We have to do a deep copy here
  // to preserve the original parameters at the minimum.
  FitParameterList FinalParameters(InitialParameters);

  std::stringstream resultsOut;
  resultsOut << "Central values of floating paramters:" << std::endl;

  for (auto finalPar : FinalParameters) {
    if (finalPar.IsFixed)
      continue;

    // central value
    double val = minState.Value(finalPar.Name);

    // shift to [-pi;pi] if parameter is a phase
    if (finalPar.Name.find("phase") != finalPar.Name.npos)
      val = shiftAngle(val);
    finalPar.Value = val;

    resultsOut << finalPar.Name << " " << val << std::endl;
  }

  LOG(DEBUG) << "MinuitIF::createResult() | " << resultsOut.str();

  Result.FinalParameters = FinalParameters;
  Result.InitialParameters = InitialParameters;

  if (minState.HasCovariance()) {
    ROOT::Minuit2::MnUserCovariance minuitCovMatrix = minState.Covariance();
    // Size of Minuit covariance vector is given by dim*(dim+1)/2.
    // dim is the dimension of the covariance matrix.
    // The dimension can therefore be calculated as
    // dim = -0.5+-0.5 sqrt(8*size+1)
    assert(minuitCovMatrix.Nrow() == NumFreeParameter);
    Result.CovarianceMatrix = std::vector<std::vector<double>>(
        NumFreeParameter, std::vector<double>(NumFreeParameter));
    for (unsigned i = 0; i < NumFreeParameter; ++i)
      for (unsigned j = i; j < NumFreeParameter; ++j) {
        Result.CovarianceMatrix.at(i).at(j) = minuitCovMatrix(j, i);
        Result.CovarianceMatrix.at(j).at(i) =
            minuitCovMatrix(j, i); // fill lower half
      }

  } else {
    LOG(ERROR)
        << "MinuitIF::createResult(): no valid correlation matrix available!";
  }
  if (minState.HasGlobalCC()) {
    Result.GlobalCC = minState.GlobalCC().GlobalCC();
  } else {
    Result.GlobalCC = std::vector<double>(NumFreeParameter, 0);
    LOG(ERROR)
        << "MinuitIF::createResult(): no valid global correlation available!";
  }

  Result.FinalEstimatorValue = minState.Fval();
  Result.Edm = minState.Edm();
  Result.IsValid = min.IsValid();
  Result.CovPosDef = min.HasPosDefCovar();
  Result.HasValidParameters = min.HasValidParameters();
  Result.HasValidCov = min.HasValidCovariance();
  Result.HasAccCov = min.HasAccurateCovar();
  Result.HasReachedCallLimit = min.HasReachedCallLimit();
  Result.EdmAboveMax = min.IsAboveMaxEdm();
  Result.HesseFailed = min.HesseFailed();
  Result.ErrorDef = min.Up();
  Result.NFcn = min.NFcn();

  return Result;
}
