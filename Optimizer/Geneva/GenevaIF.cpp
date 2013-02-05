#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include "Optimizer/Geneva/GenevaIF.hpp"
#include "Optimizer/Geneva/GStartIndividual.hpp"
#include "Core/PWAParameter.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;

GenevaIF::GenevaIF(std::shared_ptr<ControlParameter> theData, std::string inConfigFile, boost::uint16_t inparallelizationMode, bool inserverMode, std::string inip, unsigned short inport, Gem::Common::serializationMode inserMode) : _myData(theData),configFile(inConfigFile),parallelizationMode(inparallelizationMode),serverMode(inserverMode),ip(inip),port(inport),serMode(inserMode){
  bool parsedConfig = parseConfigFile(configFile,
		      nProducerThreads,
		      nEvaluationThreads,
		      populationSize,
		      nParents,
		      maxIterations,
		      maxMinutes,
		      reportIteration,
		      rScheme,
		      smode,
		      arraySize,
		      processingCycles,
		      returnRegardless,
		      waitFactor);
    if(!parsedConfig) std::cout << "TODO" << std::endl;
}

GenevaIF::~GenevaIF()
{
  //std::cout << "GenevaIF::~GenevaIF: I'll be back" << std::endl;
  //delete _myFcn;
}

const double GenevaIF::exec(std::vector<std::shared_ptr<PWAParameter> >& par){
  // create par arrays
  unsigned int NPar = par.size();
  double val[NPar], min[NPar], max[NPar], err[NPar];
  for(unsigned int i=0; i<NPar; i++){
      val[i] = par[i]->GetValue();
      min[i] = par[i]->GetMinValue();
      max[i] = par[i]->GetMaxValue();
      err[i] = par[i]->GetError();
  }

  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
  GRANDOMFACTORY->setArraySize(arraySize);

  //***************************************************************************
  // If this is a client in networked mode, we can just start the listener and
  // return when it has finished
  if(parallelizationMode==2 && !serverMode) {
    boost::shared_ptr<GAsioTCPClientT<GIndividual> > p(new GAsioTCPClientT<GIndividual>(ip, boost::lexical_cast<std::string>(port)));

    p->setMaxStalls(0); // An infinite number of stalled data retrievals
    p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

    // Prevent return of unsuccessful adaption attempts to the server
    p->returnResultIfUnsuccessful(returnRegardless);

    // Start the actual processing loop
    p->run();

    return 0;
  }
  //***************************************************************************

 // Create the first set of parent individuals. Initialization of parameters is (should be) done randomly.
  std::vector<boost::shared_ptr<GStartIndividual> > parentIndividuals;
  for(std::size_t p = 0 ; p<nParents; p++) {
    //TODO: vary start parameters
    boost::shared_ptr<GStartIndividual> gdii_ptr(new GStartIndividual(_myData, NPar, val, min, max, err));
    gdii_ptr->setProcessingCycles(processingCycles);

    parentIndividuals.push_back(gdii_ptr);
  }

  //***************************************************************************
  // We can now start creating populations. We refer to them through the base class

  // This smart pointer will hold the different population types
  boost::shared_ptr<GEvolutionaryAlgorithm> pop_ptr;

  // Create the actual populations
  switch (parallelizationMode) {
    //-----------------------------------------------------------------------------------------------------
  case 0: // Serial execution
    // Create an empty population
    pop_ptr = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());
    break;

    //-----------------------------------------------------------------------------------------------------
  case 1: // Multi-threaded execution
    {
      // Create the multi-threaded population
      boost::shared_ptr<GMultiThreadedEA> popPar_ptr(new GMultiThreadedEA());

      // Population-specific settings
      popPar_ptr->setNThreads(nEvaluationThreads);

      // Assignment to the base pointer
      pop_ptr = popPar_ptr;
    }
    break;

    //-----------------------------------------------------------------------------------------------------
  case 2: // Networked execution (server-side)
    {
      // Create a network consumer and enrol it with the broker
      boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port));
      gatc->setSerializationMode(serMode);
      GINDIVIDUALBROKER->enrol(gatc);

      // Create the actual broker population
      boost::shared_ptr<GBrokerEA> popBroker_ptr(new GBrokerEA());
      popBroker_ptr->setWaitFactor(waitFactor);

      // Assignment to the base pointer
      pop_ptr = popBroker_ptr;
    }
    break;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Now we have suitable populations and can fill them with data

  // Add individuals to the population
  for(std::size_t p = 0 ; p<nParents; p++) {
    pop_ptr->push_back(parentIndividuals[p]);
  }

  // Specify some general population settings
  pop_ptr->setDefaultPopulationSize(populationSize,nParents);
  pop_ptr->setMaxIteration(maxIterations);
  pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes));
  pop_ptr->setReportIteration(reportIteration);
  pop_ptr->setRecombinationMethod(rScheme);
  pop_ptr->setSortingScheme(smode);

  // Do the actual optimization
  pop_ptr->optimize();

  //--------------------------------------------------------------------------------------------

  boost::shared_ptr<GStartIndividual> bestIndividual_ptr=pop_ptr->getBestIndividual<GStartIndividual>();

  std::vector<std::shared_ptr<PWAParameter> > resultPar;
  bestIndividual_ptr->getPar(resultPar);

  //par = resultPar; Geneva ignores bounds and errors!
  for(unsigned int i=0; i<par.size(); i++){  //TODO: better way, no cast or check type
    par[i]->SetValue(resultPar[i]->GetValue());
  }

  return _myData->controlParameter(resultPar);// bestIndividual_ptr->fitnessCalculation();
}