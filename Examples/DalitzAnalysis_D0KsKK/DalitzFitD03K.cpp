// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

///
/// \file
/// Executable for Dalitz plot analysis of the decay D0 -> K_S K+ K-
///

// Standard header files go here
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include <boost/program_options.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

// Root header files go here
#include "TFile.h"

// Core header files go here
#include "Core/Event.hpp"
#include "Core/Particle.hpp"
#include "Core/FitParameter.hpp"
#include "Core/ParameterList.hpp"
#include "Core/FunctionTree.hpp"
#include "Core/TableFormater.hpp"
#include "Core/FitParameter.hpp"
#include "Core/Logging.hpp"

// ComPWA header files go here
#include "DataReader/RootReader/RootReader.hpp"
#include "DataReader/RootReader/RootEfficiency.hpp"
#include "DataReader/CorrectionTable.hpp"
#include "DataReader/DataCorrection.hpp"
#include "Estimator/MinLogLH/MinLogLH.hpp"
#include "Optimizer/Minuit2/MinuitIF.hpp"
#include "Optimizer/Minuit2/MinuitResult.hpp"

// HelicityFormlism
#include "Physics/DecayDynamics/AbstractDynamicalFunction.hpp"
#include "Physics/DecayDynamics/AmpFlatteRes.hpp"
#include "Physics/DecayDynamics/NonResonant.hpp"
#include "Physics/DecayDynamics/RelativisticBreitWigner.hpp"

#include "Physics/HelicityFormalism/AmpWignerD.hpp"
#include "Physics/CoherentIntensity.hpp"
#include "Physics/SequentialPartialAmplitude.hpp"
#include "Physics/IncoherentIntensity.hpp"
#include "Physics/HelicityFormalism/HelicityDecay.hpp"
#include "Physics/HelicityFormalism/HelicityKinematics.hpp"

#include "Tools/RootGenerator.hpp"
#include "Tools/Generate.hpp"
#include "Tools/FitFractions.hpp"

#include "Tools/DalitzPlot.hpp"
#include "Tools/ParameterTools.hpp"
#include "systematics.hpp"

using namespace std;
using namespace ComPWA;
using namespace ComPWA::DataReader;
using namespace ComPWA::Physics;
using namespace ComPWA::Physics::HelicityFormalism;

// Enable serialization of MinuitResult. For some reason has to be outside
// any namespaces.
BOOST_CLASS_EXPORT(ComPWA::Optimizer::Minuit2::MinuitResult)

///
/// Dalitz plot analysis of the decay D0->K_S0 K_ K-.
///
int main(int argc, char **argv) {

  // Read command line input
  namespace po = boost::program_options;

  std::string config_file;

  po::options_description generic("Generic options");

  generic.add_options()("help,h", "produce help message");
  generic.add_options()(
      "config,c", po::value<string>(&config_file)->default_value("config.cfg"),
      "name of a file of a configuration.");

  int seed;      // initial seed
  int numEvents; // data size to be generated
  bool smearNEvents;
  std::string logLevel;
  bool disableFileLog;
  std::string outputDir;
  std::string outputFileName;

  po::options_description config("General settings");

  config.add_options()("nEvents",
                       po::value<int>(&numEvents)->default_value(1000),
                       "set of events per fit");
  config.add_options()(
      "seed", po::value<int>(&seed)->default_value(12345),
      "set random number seed; default is to used unique seed for every job");
  config.add_options()(
      "logLevel", po::value<std::string>(&logLevel)->default_value("trace"),
      "set log level: error|warning|info|debug");
  config.add_options()("disableFileLog",
                       po::value<bool>(&disableFileLog)->default_value(0),
                       "write log file? 0/1");
  config.add_options()(
      "outputFile",
      po::value<std::string>(&outputFileName)->default_value("out"),
      "set output file name x. The files x.root and .log are created");
  config.add_options()("outputDir",
                       po::value<std::string>(&outputDir)->default_value("./"),
                       "set output directory");
  config.add_options()("smearNEvents",
                       po::value<bool>(&smearNEvents)->default_value(0),
                       "smear NEvents for sqrt(NEvents)");

  std::string dataFile, dataFileTreeName;
  std::string bkgFile, bkgFileTreeName;
  std::string trueModelFile, trueBkgModelFile;
  double trueSignalFraction;
  std::string trueAmpOption;
  bool resetWeights;
  std::string inputResult;

  po::options_description config_inout("Input/Generate settings");

  config_inout.add_options()(
      "dataFile", po::value<std::string>(&dataFile)->default_value(""),
      "set input data; leave blank for toy mc generation");
  config_inout.add_options()(
      "dataFileTreeName",
      po::value<std::string>(&dataFileTreeName)->default_value("data"),
      "set tree name in data file");
  config_inout.add_options()(
      "bkgFile", po::value<std::string>(&bkgFile)->default_value(""),
      "set bkg sample; leave blank if we fit data");
  config_inout.add_options()(
      "bkgFileTreeName",
      po::value<std::string>(&bkgFileTreeName)->default_value("data"),
      "set tree name in bkg file");
  config_inout.add_options()(
      "trueModelFile",
      po::value<std::string>(&trueModelFile)->default_value("model.xml"),
      "set XML model file of true distribution");
  config_inout.add_options()(
      "trueBkgFile",
      po::value<std::string>(&trueBkgModelFile)->default_value(""),
      "set XML model file of true distribution");
  config_inout.add_options()(
      "trueSignalFraction",
      po::value<double>(&trueSignalFraction)->default_value(1.),
      "add this number of background events: Nbkg=(1-f)*NSignal; only makes "
      "sense together with bkgFile; ");
  config_inout.add_options()(
      "trueAmpOption",
      po::value<std::string>(&trueAmpOption)->default_value(""),
      "Use nocorrection=2/tagged=1/untagged=0");
  config_inout.add_options()("resetWeights",
                             po::value<bool>(&resetWeights)->default_value(0),
                             "should we reset all weights to one?");
  config_inout.add_options()(
      "inputResult", po::value<std::string>(&inputResult)->default_value(""),
      "input file for MinuitResult");

  std::string efficiencyFile;
  std::string efficiencyObject;
  std::string phspEfficiencyFile;
  std::string phspEfficiencyFileTreeName;
  std::string phspEfficiencyFileTrueTreeName;
  bool applySysCorrection;
  po::options_description config_eff("Efficiency settings");
  config_eff.add_options()(
      "efficiencyFile",
      po::value<std::string>(&efficiencyFile)->default_value(""),
      "root file with binned efficiency class. If none is specified uniform "
      "efficiency is assumed.")("efficiencyObject",
                                po::value<std::string>(&efficiencyObject)
                                    ->default_value("efficiency_m23cosTheta23"),
                                "name of TObject inside efficiencyFile")(
      "phspEfficiencyFile",
      po::value<std::string>(&phspEfficiencyFile)->default_value(""),
      "root file with reconstructed unbinned data from flat PHSP sample. We "
      "use this for "
      "an unbinned efficiency correction. Leave blank to use binned "
      "efficiency")(
      "phspEfficiencyFileTreeName",
      po::value<std::string>(&phspEfficiencyFileTreeName)->default_value("mc"),
      "set tree name for unbinned efficiency correction")(
      "phspEfficiencyFileTrueTreeName",
      po::value<std::string>(&phspEfficiencyFileTrueTreeName)
          ->default_value(""),
      "set tree name for tree with true values")(
      "applySysCorrection",
      po::value<bool>(&applySysCorrection)->default_value(0),
      "apply correction for data/MC difference");

  unsigned int mcPrecision;
  unsigned int ampMcPrecision;
  bool usePreFitter;
  bool useMinos;
  bool useHesse;
  bool useRandomStartValues;
  bool calculateInterference;
  int fitFractionError;
  double fitSignalFraction;
  std::string fitModelFile;
  std::string fitBkgFile;
  std::string fittingMethod;
  std::string ampOption;
  double penaltyScale;
  po::options_description config_fit("Fit settings");
  config_fit.add_options()(
      "mcPrecision",
      po::value<unsigned int>(&mcPrecision)->default_value(100000),
      "Precision for MC integration and normalization")(
      "ampMcPrecision",
      po::value<unsigned int>(&ampMcPrecision)->default_value(0),
      "Precision for MC integration and normalization")(
      "fittingMethod",
      po::value<std::string>(&fittingMethod)->default_value("tree"),
      "choose between 'tree', 'amplitude' and 'plotOnly'")(
      "useMinos", po::value<bool>(&useMinos)->default_value(0),
      "Run MINOS for each parameter")(
      "useHesse", po::value<bool>(&useHesse)->default_value(1),
      "Run HESSE after MIGRAD")(
      "fitModelFile",
      po::value<std::string>(&fitModelFile)->default_value("model.xml"),
      "Set XML model file of fit distribution")(
      "fitBkgFile", po::value<std::string>(&fitBkgFile)->default_value(""),
      "Set XML background model file")(
      "ampOption", po::value<std::string>(&ampOption)->default_value("none"),
      "Use none/tagged/taggedSym/untagged")(
      "fitSignalFraction",
      po::value<double>(&fitSignalFraction)->default_value(1.),
      "Signal fraction")("usePreFitter",
                         po::value<bool>(&usePreFitter)->default_value(0),
                         "Run Geneva as prefitter")(
      "useRandomStartValues",
      po::value<bool>(&useRandomStartValues)->default_value(0),
      "Randomize start values")(
      "penaltyScale", po::value<double>(&penaltyScale)->default_value(0))(
      "calculateInterference",
      po::value<bool>(&calculateInterference)->default_value(0),
      "Calculate interference terms")(
      "fitFractionError", po::value<int>(&fitFractionError)->default_value(0),
      "Fit fraction errors are calculated by Monte-Carlo approach. 0 = "
      "disabled, >0 = MC precision");

  bool enablePlotting;
  bool DalitzPlotSample;
  int plotSize;
  bool plotCorrectEfficiency;
  int plotNBins;
  bool plotAmplitude;
  po::options_description config_plot("Plot settings");
  config_plot.add_options()("plotting",
                            po::value<bool>(&enablePlotting)->default_value(1),
                            "Enable/Disable plotting")(
      "DalitzPlot", po::value<bool>(&DalitzPlotSample)->default_value(1),
      "Enable/Disable plotting of data")(
      "plotSize", po::value<int>(&plotSize)->default_value(100000),
      "Size of sample for amplitude plot")(
      "plotNBins", po::value<int>(&plotNBins)->default_value(100),
      "Number of bins")(
      "plotCorrectEfficiency",
      po::value<bool>(&plotCorrectEfficiency)->default_value(0),
      "Number of bins")("plotAmplitude",
                        po::value<bool>(&plotAmplitude)->default_value(1),
                        "Enable/Disable plotting of amplitude");

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(config_inout);
  cmdline_options.add(config_eff).add(config_fit).add(config_plot);
  po::options_description config_file_options;
  config_file_options.add(config).add(config_inout).add(config_eff);
  config_file_options.add(config_fit).add(config_plot);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, cmdline_options), vm);

  po::positional_options_description p;
  store(po::command_line_parser(argc, argv)
            .options(cmdline_options)
            .positional(p)
            .run(),
        vm);
  notify(vm);
  if (vm.count("help")) {
    cout << cmdline_options << "\n";
    return 1;
  }

  ifstream ifs(config_file.c_str());
  if (ifs) {
    store(parse_config_file(ifs, config_file_options), vm);
    notify(vm);
  }
  std::string fileNamePrefix = outputDir + std::string("/") + outputFileName;
  std::string logFileName = fileNamePrefix + std::string(".log");
  if (disableFileLog)
    logFileName = "";

  Logging log(logFileName, boost::log::trivial::info); // initialize logging
  log.setLogLevel(logLevel);

  // check configuration
  assert(!outputDir.empty());
  if (trueModelFile.empty()) {
    LOG(error) << "True model file not set";
    trueModelFile = fitModelFile;
  }
  if (fitModelFile.empty() && fittingMethod != "plotOnly") {
    LOG(error) << "Fit model file not set";
  }
  if (fittingMethod != "tree" && fittingMethod != "amplitude" &&
      fittingMethod != "plotOnly") {
    LOG(error) << "Unknown fitting method: " << fittingMethod;
    return 1;
  }
  po::notify(vm);

  if (ampMcPrecision == 0)
    ampMcPrecision = mcPrecision;

  boost::property_tree::ptree fitModelTree;
  boost::property_tree::xml_parser::read_xml(fitModelFile, fitModelTree);
  boost::property_tree::ptree trueModelTree;
  boost::property_tree::xml_parser::read_xml(trueModelFile, trueModelTree);

  auto fitModelPartL = std::make_shared<ComPWA::PartList>();
  auto trueModelPartL = std::make_shared<ComPWA::PartList>();
  ReadParticles(fitModelPartL, fitModelTree);
  ReadParticles(trueModelPartL, trueModelTree);

  // initialize kinematics of decay
  auto fitModelKin = std::make_shared<HelicityKinematics>(
      fitModelPartL, fitModelTree.get_child("HelicityKinematics"));
  auto trueModelKin = std::make_shared<HelicityKinematics>(
      trueModelPartL, trueModelTree.get_child("HelicityKinematics"));

  // Initialize random generator useing trueModel parameters
  std::shared_ptr<Generator> gen = std::shared_ptr<Generator>(
      new Tools::RootGenerator(trueModelPartL, trueModelKin->initialState(),
                               trueModelKin->finalState(), seed));

  // EFFICIENCY
  auto eff = std::shared_ptr<Efficiency>(new UnitEfficiency());

  // Binned efficiency
  if (!efficiencyFile.empty()) {
    TFile *tf = new TFile(TString(efficiencyFile));
    TH1 *h_passed = (TH1 *)tf->Get(TString(efficiencyObject) + "_passed");
    TH1 *h_total = (TH1 *)tf->Get(TString(efficiencyObject) + "_total");
    DataReader::RootEfficiency rootEff(h_passed, h_total);
    tf->Close();
    auto eff = std::make_shared<ComPWA::DataReader::RootEfficiency>(rootEff);
  }

  // Unbinned efficiency
  std::shared_ptr<Data> phspData, phspTrueData;
  if (!phspEfficiencyFile.empty()) {
    // sample with accepted phsp events
    phspData = std::shared_ptr<Data>(new RootReader(
        phspEfficiencyFile, phspEfficiencyFileTreeName, mcPrecision));
    phspData->reduceToPhsp(trueModelKin);
  }

  // Construct intensities
  auto intens = std::make_shared<IncoherentIntensity>(
      fitModelPartL, fitModelKin, fitModelTree.get_child("Intensity"));

  auto trueIntens = std::make_shared<IncoherentIntensity>(
      trueModelPartL, trueModelKin, trueModelTree.get_child("Intensity"));

  // Generation of toy phase space sample
  std::shared_ptr<Data> toyPhspData(new RootReader()); // Toy sample
  ComPWA::Tools::generatePhsp(mcPrecision, gen, toyPhspData);
  // set efficiency values for each event
  toyPhspData->setEfficiency(trueModelKin, eff);
  if (!phspData) // in case no phase space sample is provided we use the toy sample
    phspData = toyPhspData;
  
  // Setting samples for normalization
  auto toyPoints = std::make_shared<std::vector<DataPoint>>(
      toyPhspData->dataPoints(trueModelKin));
  auto phspPoints = toyPoints;
  if (phspData) {
    auto phspPoints = std::make_shared<std::vector<DataPoint>>(
        phspData->dataPoints(trueModelKin));
  }
  trueIntens->setPhspSample(phspPoints, toyPoints);

  toyPoints = std::make_shared<std::vector<DataPoint>>(
      toyPhspData->dataPoints(fitModelKin));
  phspPoints = toyPoints;
  if (phspData) {
    auto phspPoints = std::make_shared<std::vector<DataPoint>>(
        phspData->dataPoints(fitModelKin));
  }
  intens->setPhspSample(phspPoints, toyPoints);

  //======================= READING DATA =============================
  // Sample is used for minimization
  std::shared_ptr<Data> sample(new Data());

  // Temporary samples for signal and background
  std::shared_ptr<Data> inputData, inputBkg;

  // Smear total number of events
  if (smearNEvents && numEvents > 0)
    numEvents += (int)gen->gauss(0, sqrt(numEvents));

  if (!dataFile.empty()) {
    int numSignalEvents = numEvents;
    //    if (inputBkg)
    //      numSignalEvents -= inputBkg->getNEvents();
    LOG(info) << "Reading data file...";
    std::shared_ptr<Data> inD(new RootReader(dataFile, dataFileTreeName));
    inD->reduceToPhsp(trueModelKin);
    if (resetWeights)
      inD->resetWeights(); // resetting weights if requested
    inputData = inD->rndSubSet(trueModelKin, numSignalEvents, gen);
    inputData->setEfficiency(trueModelKin, eff);
    // run.SetData(inputData);
    inD = std::shared_ptr<Data>();
    sample->append(*inputData);
  }
  
  //========== Generation of data sample ===========
  // generation with unbinned efficiency correction - use full sample
  if (!phspEfficiencyFile.empty() && (!inputData || !inputBkg)) {
    // assume that efficiency of hit&miss is large 0.5%
    double phspSampleSize = numEvents / 0.005;

    std::shared_ptr<Data> fullPhsp(new RootReader(
        phspEfficiencyFile, phspEfficiencyFileTreeName, phspSampleSize));
    if (applySysCorrection) {
      MomentumCorrection *trkSys = getTrackingCorrection();
      MomentumCorrection *pidSys = getPidCorrection();
      trkSys->print();
      pidSys->print();
      fullPhsp->applyCorrection(*trkSys);
      fullPhsp->applyCorrection(*pidSys);
    }
    std::shared_ptr<Data> fullTruePhsp;
    if (!phspEfficiencyFileTrueTreeName.empty()) {
      fullTruePhsp = std::shared_ptr<Data>(new RootReader(
          phspEfficiencyFile, phspEfficiencyFileTrueTreeName, phspSampleSize));
      // Data* fullPhspRed = fullPhsp->EmptyClone();
      // Data* fullPhspTrueRed = fullTruePhsp->EmptyClone();
      // rndReduceSet(phspSampleSize,gen,fullPhsp.get(),fullPhspRed,
      // fullTruePhsp.get(),fullPhspTrueRed);
      // fullPhsp = std::shared_ptr<Data>(fullPhspRed);
      // fullTruePhsp = std::shared_ptr<Data>(fullPhspTrueRed);
    } else {
      // Data* fullPhspRed = fullPhsp->EmptyClone();
      // rndReduceSet(phspSampleSize,gen,fullPhsp.get(),fullPhspRed);
      // fullPhsp = std::shared_ptr<Data>(fullPhspRed);
    }
  }
  if (!inputData) {
    LOG(info) << "Generating sample!";
    //    ComPWA::Tools::generate(numEvents, trueModelKin, gen, trueIntens,
    //    sample, phspData, toyPhspData);
    // Pass Null pointers for phsp sample which are than generated on the fly
    ComPWA::Tools::generate(numEvents, trueModelKin, gen, trueIntens, sample,
                            std::shared_ptr<Data>(), std::shared_ptr<Data>());
    LOG(info) << "Sample size: " << sample->numEvents();
  }
  // Reset phsp sample to save memory
  // run.SetPhspSample(std::shared_ptr<Data>());

  LOG(info) << "Subsystems used by true model:";
  for (auto i : trueModelKin->subSystems()) {
    // Have to add " " here (bug in boost 1.59)
    LOG(info) << " " << i;
  }
  std::stringstream s;
  s << "Printing the first 10 events of data sample:\n";
  for (int i = 0; (i < sample->numEvents() && i < 10); ++i) {
    DataPoint p;
    trueModelKin->convert(sample->event(i), p);
    s << p << "\n";
  }
  LOG(info) << s.str();

  // sample->writeData("test.root","tr");
  sample->reduceToPhsp(trueModelKin);

  LOG(info) << "================== SETTINGS =================== ";

  LOG(info) << "==== GENERAL";
  LOG(info) << "Number of events: " << numEvents;
  LOG(info) << "Initial seed: " << seed;
  LOG(info) << "Log level: " << logLevel;
  LOG(info) << "Output file: " << fileNamePrefix << "-* [*.root,*.pdf,*.tex]";
  LOG(info) << "==== INPUT/GENERATION";
  if (!dataFile.empty()) { // read in data
    LOG(info) << "Data file: " << dataFile << " [tree=" << dataFileTreeName
              << " ]";
  } else { // generate signal
    LOG(info) << "true signal fraction: " << trueSignalFraction;
    LOG(info) << "True model file: " << trueModelFile;
    LOG(info) << "True amplitude option: " << trueAmpOption;
  }
  if (trueSignalFraction != 1.0) {
    if (!bkgFile.empty()) { // read in background
      LOG(info) << "Background file: " << bkgFile
                << " [tree=" << bkgFileTreeName << " ]";
    } else { // generate background
      LOG(info) << "True background model file: " << trueBkgModelFile;
    }
  }
  LOG(info) << "Total events in input sample: " << sample->numEvents();

  LOG(info) << "==== EFFICIENCY";
  if (!efficiencyFile.empty())
    LOG(info) << "Binned Efficiency file: " << efficiencyFile
              << " [obj=" << efficiencyObject << "]";
  if (!phspEfficiencyFile.empty()) {
    LOG(info) << "PHSP data input file: " << phspEfficiencyFile
              << " [tree=" << phspEfficiencyFileTreeName << "]";
    if (!phspEfficiencyFileTrueTreeName.empty())
      LOG(info) << "True tree name: " << phspEfficiencyFileTrueTreeName;
    if (applySysCorrection)
      LOG(info) << "Unbinned efficiency is corrected "
                   "for tracking systematics!";
    LOG(info) << "Unbinned efficiency correction is used!";
  }
  if (phspEfficiencyFile.empty() && !efficiencyFile.empty())
    LOG(info) << "Binned efficiency correction is used!";
  if (!phspEfficiencyFile.empty() && !efficiencyFile.empty())
    LOG(info) << "No efficiency correction!";

  LOG(info) << "==== FIT";
  LOG(info) << "MC precision: " << mcPrecision;
  LOG(info) << "Amplitude MC precision: " << ampMcPrecision;
  LOG(info) << "Fitting method: " << fittingMethod;
  LOG(info) << "Fit signal fraction: " << fitSignalFraction;
  LOG(info) << "Amplitude option: " << ampOption;
  LOG(info) << "Fit model file: " << fitModelFile;
  if (!fitBkgFile.empty())
    LOG(info) << "Fit bkg model file: " << fitBkgFile;
  LOG(info) << "Using Geneva as pre fitter: " << usePreFitter;
  LOG(info) << "Use MINOS: " << useMinos;
  LOG(info) << "Accurate errors on fit fractions: " << fitFractionError;
  LOG(info) << "Penalty scale: " << penaltyScale;

  LOG(info) << "==== PLOTTING";
  LOG(info) << "enable plotting: " << enablePlotting;
  if (enablePlotting) {
    LOG(info) << "plotting size: " << plotSize;
    LOG(info) << "number of bins: " << plotNBins;
    LOG(info) << "Correct samples for efficiency: " << plotCorrectEfficiency;
    LOG(info) << "DalitzPlotSample: " << DalitzPlotSample;
    LOG(info) << "plotAmplitude: " << plotAmplitude;
  }
  LOG(info) << "===============================================";

  std::shared_ptr<FitResult> result;
  ParameterList finalParList;
  if (fittingMethod != "plotOnly") {
    //========================FITTING =====================
    ParameterList truePar, fitPar;
    trueIntens->parameters(truePar);
    intens->parameters(fitPar);
    LOG(debug) << fitPar.to_str();

    //    std::cout << "phi = "
    //    <<Tools::Integral(intens->component("phi(1020)"), toyPoints,
    //                                 fitModelKin->phspVolume())
    //              << std::endl;
    //
    //    std::cout << "a0 = " <<Tools::Integral(intens->component("a0(980)0"),
    //    toyPoints,
    //                                 fitModelKin->phspVolume())
    //              << std::endl;
    //    std::cout << "total =
    //    "<<Tools::Integral(intens->component("D0toKSK+K-"), toyPoints,
    //                                 fitModelKin->phspVolume())
    //              << std::endl;
    //    std::cout << "ff= " << Tools::CalculateFitFraction(
    //                     fitModelKin, intens, toyPoints,
    //                     std::make_pair("phi(1020)", "D0toKSK+K-"))
    //              << std::endl;
    //=== Constructing likelihood
    auto esti = std::make_shared<Estimator::MinLogLH>(
        fitModelKin, intens, sample, toyPhspData, phspData, 0, 0);

    std::cout.setf(std::ios::unitbuf);
    if (fittingMethod == "tree") {
      esti->UseFunctionTree(true);
      esti->tree()->parameter();
      LOG(debug) << esti->tree()->head()->print(25);
    }

    if (useRandomStartValues)
      randomStartValues(fitPar);
    LOG(debug) << "Initial LH=" << esti->controlParameter(fitPar) << ".";

    // Set start error of 0.05 for parameters
    setErrorOnParameterList(fitPar, 0.05, useMinos);

    auto minuitif = new Optimizer::Minuit2::MinuitIF(esti, fitPar);
    minuitif->setUseHesse(useHesse);
    LOG(info) << "Norm: " << intens->intensity(phspPoints->at(0));
    // Start minimization
    result = minuitif->exec(fitPar);
    finalParList = result->finalParameters();

    // Calculation of fit fractions
    std::vector<std::pair<std::string, std::string>> fitComponents;
    fitComponents.push_back(
        std::pair<std::string, std::string>("phi(1020)", "D0toKSK+K-"));
    fitComponents.push_back(
        std::pair<std::string, std::string>("a0(980)0", "D0toKSK+K-"));
    fitComponents.push_back(
        std::pair<std::string, std::string>("a0(980)+", "D0toKSK+K-"));
    fitComponents.push_back(
        std::pair<std::string, std::string>("a2(1320)-", "D0toKSK+K-"));
    ParameterList ff = Tools::CalculateFitFractions(
        fitModelKin, intens->component("D0toKSK+K-"), toyPoints, fitComponents);
    Tools::CalcFractionError(
        fitPar,
        std::dynamic_pointer_cast<ComPWA::Optimizer::Minuit2::MinuitResult>(
            result)
            ->covarianceMatrix(),
        ff, fitModelKin, intens->component("D0toKSK+K-"), toyPoints, 20,
        fitComponents);
    result->setFitFractions(ff);

    // Print fit result
    result->print();

    // Save fit result
    std::ofstream ofs(fileNamePrefix + std::string("-fitResult.xml"));
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(result);

    if (!disableFileLog) { // Write fit result
      // Save final amplitude
      boost::property_tree::ptree ptout;
      ptout.add_child("IncoherentIntensity", intens->save());
      boost::property_tree::xml_parser::write_xml(
          fileNamePrefix + std::string("-Model.xml"), ptout, std::locale());
      // Does not compile with boost1.54
      //      boost::property_tree::xml_parser::write_xml(
      //          fileNamePrefix + std::string("-Model.xml"), ptout,
      //          std::locale(),
      //          boost::property_tree::xml_writer_make_settings<std::string>('
      //          ', 4, "utf-8"));

      //      LOG(info) << "Average resonance width of fit model: "
      //                << fitAmpSum->averageWidth();
    } else if (!inputResult.empty()) {
      LOG(info) << "Reading MinuitResult from " << inputResult;
      std::shared_ptr<Optimizer::Minuit2::MinuitResult> inResult;
      std::ifstream ifs(inputResult);
      if (!ifs.good())
        throw std::runtime_error("input stream not good!");
      boost::archive::xml_iarchive ia(ifs);
      ia >> BOOST_SERIALIZATION_NVP(inResult);
      ifs.close();

      //    inResult->SetUseCorrelatedErrors(fitFractionError);
      if (calculateInterference)
        inResult->setCalcInterference(0);
      //    inResult->SetFitFractions(ParameterList()); // reset fractions list
      result = inResult;
      result->print();
    }

    // Fill final parameters if minimization was not run
    if (!finalParList.numParameters()) {
      ParameterList tmpList;
      //    Amplitude::FillAmpParameterToList(ampVec, tmpList);
      finalParList.DeepCopy(tmpList);
    }

    //======================= PLOTTING =============================
    if (enablePlotting) {
      std::shared_ptr<Data> pl_phspSample(new RootReader());
      LOG(info) << "Plotting results...";
      if (!phspEfficiencyFile.empty()) { // unbinned plotting
                                         // sample with accepted phsp events
        pl_phspSample = std::shared_ptr<Data>(new RootReader(
            phspEfficiencyFile, phspEfficiencyFileTreeName, plotSize));

        std::shared_ptr<Data> plotTruePhsp;
        if (!phspEfficiencyFileTrueTreeName.empty())
          plotTruePhsp = std::shared_ptr<Data>(new RootReader(
              phspEfficiencyFile, phspEfficiencyFileTrueTreeName, plotSize));
      } else {
        ComPWA::Tools::generatePhsp(plotSize, gen, pl_phspSample);
      }
      // reduce sample to phsp
      pl_phspSample->reduceToPhsp(trueModelKin);
      pl_phspSample->setEfficiency(trueModelKin, eff);

      //-------- Instance of DalitzPlot
      ComPWA::Tools::DalitzPlot pl(fitModelKin, fileNamePrefix, plotNBins);
      // set data sample
      pl.setData(sample);
      // set phsp sample
      pl.setPhspData(pl_phspSample);
      // set amplitude
      pl.setFitAmp(intens, "", kBlue - 4);
      // select components to plot
      pl.drawComponent("D0toKSK+K-", "D0toKSK+K-", "Signal", kGreen);
      pl.drawComponent("a0(980)0", "D0toKSK+K-", "a_{0}(980)^{0}", kMagenta);
      pl.drawComponent("a0(980)+", "D0toKSK+K-", "a_{0}(980)^{+}", kMagenta+2);
      pl.drawComponent("phi(1020)", "D0toKSK+K-", "#phi(1020)", kMagenta+4);
      pl.drawComponent("BkgD0toKSK+K-", "BkgD0toKSK+K-", "Background", kRed);

      // Fill histograms and create canvases
      pl.plot();
    }
    LOG(info) << "FINISHED!";

    // Exit code is exit code of fit routine. 0 is good/ 1 is bad
    if (result)
      return result->hasFailed();
    return 0;
  }
}