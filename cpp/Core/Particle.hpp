// Copyright (c) 2015, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

///
/// \file
/// Particle and FourMomentum class.
///

#ifndef _PARTICLE_HPP_
#define _PARTICLE_HPP_

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <boost/property_tree/ptree.hpp>

namespace ComPWA {

///
/// \class FourMomentum
/// ComPWA four momentum class.
///
class FourMomentum {

public:
  FourMomentum(double px, double py, double pz, double E)
      : P4(std::array<double, 4>{{px, py, pz, E}}) {}

  FourMomentum(std::array<double, 4> p4) : P4(p4) {}

  FourMomentum(std::vector<double> p4) {
    if (p4.size() != 4)
      throw std::runtime_error(
          "FourMomentum::Fourmomentum() | Size of vector not equal 4!");
    P4 = std::array<double, 4>{{p4.at(0), p4.at(1), p4.at(2), p4.at(3)}};
  }

  FourMomentum operator+(const FourMomentum &pB) const {
    // TODO: implement this fast
  }

  void operator+=(const FourMomentum &pB) {
    // TODO: implement this fast
  }

  bool operator==(const FourMomentum &pB) const { return P4 == pB.P4; }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const FourMomentum &p4) {
    stream << "(" << p4.P4[0] << "," << p4.P4[1] << "," << p4.P4[2] << ","
           << p4.P4[3] << ")";
    return stream;
  }

  double invMassSq() const { return invariantMassSq(*this); }

  double invMass() const { return std::sqrt(invMassSq()); }

  static double invariantMassSq(const FourMomentum &p4) {
    auto vec = p4.P4;
    return (vec[3] * vec[3]) - threeMomentumSq(p4);
  }

  static double threeMomentumSq(const FourMomentum &p4) {
    auto vec = p4.P4;
    return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
  }

private:
  std::array<double, 4> P4;
};

///
/// \class Particle
/// ComPWA particle class.
/// This class provides a internal container for information of a particle. The
/// class provides the momentum 4-vector and pid of the particle.
///
// TODO: this maybe has to be a class
// (because pid and four momentum are not freely changeable...)
struct Particle {
public:
  Particle(double inPx = 0, double inPy = 0, double inPz = 0, double inE = 0,
           int inpid = 0);
  Particle(const Particle &in);
  FourMomentum P4;
  int ParticleID;
};

} // namespace ComPWA
#endif
