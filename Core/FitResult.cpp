
// Copyright (c) 2015, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#include "Core/FitResult.hpp"

#include "Core/Logging.hpp"
#include "Core/TableFormatter.hpp"
#include "Core/Utils.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <iomanip>

namespace ComPWA {

std::string makeFitParameterString(ComPWA::FitParameter<double> p) {
  std::stringstream ss;
  ss << std::setprecision(10);
  ss << p.Value;
  ss << std::setprecision(5);
  if (p.IsFixed) {
    ss << " (fixed)";
  } else if (p.Error.first == p.Error.second)
    ss << " +- " << p.Error.first;
  else {
    ss << " [" << p.Error.first << ", " << p.Error.second << "]";
  }
  return ss.str();
}

void FitResult::print(std::ostream &os) const {
  auto OldPrecision(os.precision());

  using ComPWA::TableFormatter;

  os << "\n--------------FIT RESULT INFOS----------------\n";
  // ----------------- general info -----------------
  os << "initial estimator value: " << InitialEstimatorValue << "\n";
  os << "final estimator value: " << FinalEstimatorValue << "\n";
  os << "duration of fit (in seconds): " << FitDuration.count() << "\n";

  // ----------------- fit parameters -----------------
  TableFormatter FitParametersFormatter(&os);
  // Column width for parameter with symmetric error
  size_t ParErrorWidth = 30;

  // Any parameter with asymmetric errors?
  for (auto x : FinalParameters) {
    if (x.Error.first != x.Error.second) {
      ParErrorWidth = 40;
      break;
    }
  }

  FitParametersFormatter.addColumn("Nr");
  FitParametersFormatter.addColumn("Name", 30);
  FitParametersFormatter.addColumn("Initial Value", 15);
  FitParametersFormatter.addColumn("Final Value", ParErrorWidth);

  FitParametersFormatter.header();

  os << std::setprecision(10);
  size_t parameterId = 0;
  for (auto p : FinalParameters) {
    auto res = std::find_if(InitialParameters.begin(), InitialParameters.end(),
                            [&p](const ComPWA::FitParameter<double> &x) {
                              return p.Name == x.Name;
                            });

    // Is parameter an angle?
    bool IsAngle(false);
    if (p.Name.find("phase") != std::string::npos)
      IsAngle = true;

    // Print parameter name
    FitParametersFormatter << parameterId << p.Name;
    parameterId++;

    // Print initial values
    if (res != InitialParameters.end()) {
      if (IsAngle)
        FitParametersFormatter << ComPWA::Utils::shiftAngle(res->Value);
      else
        FitParametersFormatter << res->Value;
    } else {
      LOG(WARNING)
          << "FitResult operator<<(): could not find initial parameter. "
             "FitResult corrupted, skipping initial value";
      FitParametersFormatter << " "; // print blank into that table cell
    }

    // Print final value
    if (IsAngle)
      p.Value = ComPWA::Utils::shiftAngle(p.Value);
    FitParametersFormatter << makeFitParameterString(p);
  }

  FitParametersFormatter.footer();

  os << std::setprecision(5);
  // ----------------- covariance matrix -----------------
  size_t NRows(CovarianceMatrix.size());
  if (0 < NRows) {
    bool CovarianceValid(true);
    for (auto x : CovarianceMatrix) {
      if (x.size() != NRows) {
        CovarianceValid = false;
        LOG(WARNING)
            << "FitResult operator<<(): Covariance is not a square matrix!";
        break;
      }
    }
    if (CovarianceValid) {
      TableFormatter CovarianceFormatter(&os);

      // Create table structure first
      CovarianceFormatter.addColumn(" ", 17); // add empty first column
                                              // add columns first
      for (auto p : FinalParameters) {
        if (p.IsFixed)
          continue;
        CovarianceFormatter.addColumn(p.Name, 17);
      }

      // Fill table
      unsigned int n = 0;
      CovarianceFormatter.header();
      for (auto p : FinalParameters) {
        if (p.IsFixed)
          continue;

        CovarianceFormatter << p.Name;
        for (auto val : CovarianceMatrix.at(n)) {
          CovarianceFormatter << val;
        }
        n++;
      }
      CovarianceFormatter.footer();
    }
  }
  os << std::setprecision(OldPrecision); // reset os precision
} // namespace ComPWA

std::ostream &operator<<(std::ostream &os, const FitResult &Result) {
  Result.print(os);
  return os;
}
} // namespace ComPWA
