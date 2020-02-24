// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#define BOOST_TEST_MODULE Data_AsciiDataIOTest

#include "Data/Ascii/AsciiDataIO.hpp"
#include "Core/Logging.hpp"
#include "Data/EvtGen/EvtGenGenerator.hpp"
#include "Data/Generate.hpp"

#include <boost/test/unit_test.hpp>
#include <string>

#include <fstream>

namespace ComPWA {
namespace Data {
namespace Ascii {

std::vector<ComPWA::Event>
generateSample(std::size_t NumberOfEvents,
               const ComPWA::FourMomentum &CMSP4 = 1.85,
               const std::vector<double> &masses = {.5, .5, .5}) {
  auto EventGenerator = ComPWA::Data::EvtGen::EvtGenGenerator(CMSP4, masses);
  ComPWA::StdUniformRealGenerator RandomGenerator(1234);
  std::vector<ComPWA::Event> Events(NumberOfEvents);
  for (auto &Event : Events) {
    Event = EventGenerator.generate(RandomGenerator);
    Event.Weight = RandomGenerator();
  }
  return Events;
}

BOOST_AUTO_TEST_SUITE(AsciiData);

BOOST_AUTO_TEST_CASE(TestCorrectUnweighted) {
  std::string FileName = "Data_AsciiDataIOTest-CorrectUnweighted.dat";
  auto EvtList = readData(FileName);
  BOOST_CHECK_EQUAL(EvtList.Events.size(), 3);

  BOOST_CHECK_EQUAL(EvtList.Header.at(0), 123);
  BOOST_CHECK_EQUAL(EvtList.Header.at(2), -123);

  const auto &Event = EvtList.Events.front();
  BOOST_CHECK_EQUAL(Event.Weight, 1.);

  auto FourVector = Event.FourMomenta.at(0);
  BOOST_CHECK_EQUAL(FourVector.e(), 5.);
  BOOST_CHECK_EQUAL(FourVector.px(), .543);
  BOOST_CHECK_EQUAL(FourVector.py(), .2345);
  BOOST_CHECK_EQUAL(FourVector.pz(), 1.);

  FourVector = Event.FourMomenta.at(2);
  BOOST_CHECK_EQUAL(FourVector.e(), 9.);
  BOOST_CHECK_EQUAL(FourVector.px(), .85434);
  BOOST_CHECK_EQUAL(FourVector.py(), .564);
  BOOST_CHECK_EQUAL(FourVector.pz(), .923);
}

BOOST_AUTO_TEST_CASE(TestMomentumEnergyOrder) {
  std::list<std::string> TestFiles{
      "Data_AsciiDataIOTest-CorrectWeightedMomE.dat",
      "Data_AsciiDataIOTest-CorrectWeightedEmom.dat"};
  for (const auto &FileName : TestFiles) {
    auto EvtList = readData(FileName, 10);
    BOOST_CHECK_EQUAL(EvtList.Events.size(), 3);

    BOOST_CHECK_EQUAL(EvtList.Header.at(0), 123);
    BOOST_CHECK_EQUAL(EvtList.Header.at(2), -123);

    const auto &Event = EvtList.Events.front();
    BOOST_CHECK_EQUAL(Event.Weight, .77);

    auto FourVector = Event.FourMomenta.at(0);
    BOOST_CHECK_EQUAL(FourVector.e(), 5.);
    BOOST_CHECK_EQUAL(FourVector.px(), .543);
    BOOST_CHECK_EQUAL(FourVector.py(), .2345);
    BOOST_CHECK_EQUAL(FourVector.pz(), 1.);

    FourVector = Event.FourMomenta.at(2);
    BOOST_CHECK_EQUAL(FourVector.e(), 9.);
    BOOST_CHECK_EQUAL(FourVector.px(), .85434);
    BOOST_CHECK_EQUAL(FourVector.py(), .564);
    BOOST_CHECK_EQUAL(FourVector.pz(), .923);
  }
}

BOOST_AUTO_TEST_CASE(TestOverwrite) {
  const char *FileName = "Data_AsciiDataIOTest-TestOverwrite.dat";
  auto Events1 = generateSample(3);
  writeData(Events1, FileName);
  auto Events2 = generateSample(4);
  writeData(Events2, FileName, true);
  auto ImportedEvents = readData(FileName);
  BOOST_CHECK_EQUAL(ImportedEvents.Events.size(),
                    Events1.Events.size() + Events2.Events.size());
  std::remove(FileName);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Ascii
} // namespace Data
} // namespace ComPWA
