// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#include "RootDataIO.hpp"

#include "TChain.h"
#include "TClonesArray.h"
#include "TError.h" // for ignoring ROOT warnings
#include "TFile.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TParticlePDG.h"

#include "Core/Exceptions.hpp"
#include "Core/Generator.hpp"
#include "Core/Kinematics.hpp"
#include "Core/Logging.hpp"
#include "Core/Properties.hpp"

namespace ComPWA {
namespace Data {
namespace Root {

ComPWA::EventList readData(const std::string &InputFilePath,
                           const std::string &TreeName,
                           long long NumberOfEventsToRead) {
  // Ignore custom streamer warning and error message for missing trees
  auto temp_ErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kBreak;

  // Use TChain to add files through wildcards if necessary
  TChain TheChain(TreeName.c_str());
  TheChain.Add(InputFilePath.c_str());

  // Test TChain quality
  auto ListOfFiles = TheChain.GetListOfFiles();
  if (!ListOfFiles || !ListOfFiles->GetEntries())
    throw ComPWA::BadConfig("Root::readData() | Unable to load files: " +
                            InputFilePath);
  if (!TheChain.GetEntries())
    throw ComPWA::CorruptFile("Root::readData() | TTree \"" + TreeName +
                              "\" cannot be opened from file(s) " +
                              InputFilePath + "!");
  if (NumberOfEventsToRead <= 0 || NumberOfEventsToRead > TheChain.GetEntries())
    NumberOfEventsToRead = TheChain.GetEntries();
  if (!TheChain.GetBranch("Particles") || !TheChain.GetBranch("weight"))
    throw ComPWA::CorruptFile("Root::readData() | TTree does not have a "
                              "Particles and/or weight branch");

  // Set branch addresses
  TClonesArray Particles("TLorentzVector");
  TClonesArray *ParticlesPointer(&Particles);
  double Weight;
  TheChain.GetBranch("Particles")->SetAutoDelete(false);
  TheChain.SetBranchAddress("Particles", &ParticlesPointer);
  TheChain.SetBranchAddress("weight", &Weight);

  EventList EvtList;
  EvtList.Events.reserve(NumberOfEventsToRead);

  // define constant event header
  // once the root file structure is changed accordingly
  // this block of code can be exchange with something better
  Particles.Clear();
  TheChain.GetEntry(0);
  size_t IndexCounter(0);
  for (auto part = 0; part < Particles.GetEntries(); part++) {
    auto Particle = dynamic_cast<TParticle *>(Particles.At(part));
    if (!Particle)
      continue;
    EvtList.Header.insert(
        std::make_pair(IndexCounter++, Particle->GetPdgCode()));
  } // end header block

  for (Long64_t i = 0; i < NumberOfEventsToRead; ++i) {
    Particles.Clear();
    TheChain.GetEntry(i);

    Event Evt;

    auto NumberOfParticles = Particles.GetEntries();
    if (IndexCounter != NumberOfParticles) {
      LOG(ERROR) << "RootDataIO::readData(): Particle count mismatch in Event "
                 << i << "! Skipping Event...";
    }
    for (auto part = 0; part < NumberOfParticles; part++) {
      auto Particle = dynamic_cast<TParticle *>(Particles.At(part));
      if (!Particle)
        continue;
      Evt.FourMomenta.push_back(FourMomentum(
          Particle->Px(), Particle->Py(), Particle->Pz(), Particle->Energy()));
    }
    Evt.Weight = Weight;

    EvtList.Events.push_back(Evt);
  }

  gErrorIgnoreLevel = temp_ErrorIgnoreLevel;
  return EvtList;
}

void writeData(const EventList &EvtList, const std::string &OutputFileName,
               const std::string &TreeName, bool OverwriteFile) {
  // Ignore custom streamer warning
  auto temp_ErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kBreak;

  if (0 == EvtList.Events.size()) {
    LOG(ERROR) << "Root::writeData(): no events given!";
    return;
  }

  LOG(INFO) << "Root::writeData(): writing vector of " << EvtList.Events.size()
            << " events to file " << OutputFileName;

  std::string WriteFlag{"UPDATE"};
  if (OverwriteFile)
    WriteFlag = "RECREATE";

  TFile TheFile(OutputFileName.c_str(), "UPDATE");
  if (TheFile.IsZombie())
    throw std::runtime_error("Root::writeData(): can't open data file: " +
                             OutputFileName);

  // TTree branch variables
  TTree TheTree(TreeName.c_str(), TreeName.c_str());
  auto ParticlesPointer =
      new TClonesArray("TParticle", EvtList.Events[0].FourMomenta.size());
  double Weight;
  TheTree.Branch("Particles", &ParticlesPointer);
  TheTree.Branch("weight", &Weight, "weight/D");
  auto &Particles = *ParticlesPointer;

  double Mass = ComPWA::calculateInvariantMass(EvtList.Events[0]);
  TLorentzVector motherMomentum(0, 0, 0, Mass);
  for (size_t i = 0; i < EvtList.Events[0].FourMomenta.size(); ++i) {
    Particles[i] = new TParticle(EvtList.Header.at(i), 1, 0, 0, 0, 0,
                                 TLorentzVector(), motherMomentum);
  }

  for (auto const &Event : EvtList.Events) {
    Particles.Clear();
    Weight = Event.Weight;

    for (unsigned int i = 0; i < Event.FourMomenta.size(); ++i) {
      auto FourMom(Event.FourMomenta[i]);
      TLorentzVector Momentum(FourMom.px(), FourMom.py(), FourMom.pz(),
                              FourMom.e());
      auto Particle = dynamic_cast<TParticle *>(Particles.At(i));
      if (!Particle)
        continue;
      Particle->SetMomentum(Momentum);
    }
    TheTree.Fill();
  }
  TheTree.Write("", TObject::kOverwrite, 0);
  TheFile.Close();
  gErrorIgnoreLevel = temp_ErrorIgnoreLevel;
}

} // namespace Root
} // namespace Data
} // namespace ComPWA
