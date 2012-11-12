//! Wrapper of the Geneva Optimizer library.
/*! \class OIFGeneva
 * @file OIFGeneva.hpp
 * This class provides a wrapper around the Geneva library. It fulfills the
 * OIFBase interface to be easily adapted to other modules. Parameters for the
 * optimization have to be provided in a config-file, the data needs to be
 * provided with the OIFData interface.
*/

#ifndef _OIFGENEVA_HPP
#define _OIFGENEVA_HPP

#include <vector>
#include <memory>
//#include <boost/shared_ptr.hpp>
#include "OIFData.hpp"
#include "OIFBase.hpp"
#include "GArgumentParser.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// Geneva header files go here
#include <courtier/GAsioHelperFunctions.hpp>
#include <courtier/GAsioTCPClientT.hpp>
#include <courtier/GAsioTCPConsumerT.hpp>
#include <geneva/GBrokerEA.hpp>
#include <geneva/GEvolutionaryAlgorithm.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GMultiThreadedEA.hpp>
#include <common/GCommonEnums.hpp>
#include <common/GSerializationHelperFunctionsT.hpp>
#include <geneva/GOptimizationEnums.hpp>

class OIFGeneva : public OIFBase {

public:
  /// Default Constructor (0x0)
  OIFGeneva(std::shared_ptr<OIFData> theData, std::string inConfigFile="test/GStartProject.cfg", boost::uint16_t inparallelizationMode=1, bool inserverMode=false, std::string inip="localhost", unsigned short inport=10000, Gem::Common::serializationMode inserMode=Gem::Common::SERIALIZATIONMODE_TEXT);
  virtual const double exec(unsigned int Npar, double* par,  double* min, double* max, double* err); 

  /** Destructor */
  virtual ~OIFGeneva();

 protected:

 private:
  std::shared_ptr<OIFData> _myData;
 // vector<string> paramNames;

  std::string configFile;	
  boost::uint16_t parallelizationMode;
  bool serverMode;
  std::string ip;
  unsigned short port;
  Gem::Common::serializationMode serMode;
  boost::uint16_t nProducerThreads;
  boost::uint16_t nEvaluationThreads;
  std::size_t populationSize;
  std::size_t nParents;
  boost::uint32_t maxIterations;
  long maxMinutes;
  boost::uint32_t reportIteration;
  Gem::Geneva::recoScheme rScheme;
  std::size_t arraySize;
  Gem::Geneva::sortingMode smode;
  boost::uint32_t processingCycles;
  bool returnRegardless;
  boost::uint32_t waitFactor;

};

#endif /* _OIFGeneva_HPP */
