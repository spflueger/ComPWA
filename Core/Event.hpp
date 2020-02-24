// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef COMPWA_EVENT_HPP_
#define COMPWA_EVENT_HPP_

#include "Core/Particle.hpp"

#include <unordered_map>
#include <vector>

namespace ComPWA {

///
/// Data structure containing all kinematic information of a physics event.
/// The information is stored in form of a Particle list (FourMomentum).
///
struct Event {
  std::vector<FourMomentum> FourMomenta;
  double Weight = 1.0;
};

struct EventList {
  const std::vector<int> Pids = {};
  const std::vector<Event> Events = {};
};

std::ostream &operator<<(std::ostream &stream, const Event &ev);

double calculateInvariantMass(const Event &ev);

double getMaximumSampleWeight(const std::vector<Event> &sample);

} // namespace ComPWA

#endif
