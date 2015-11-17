//-------------------------------------------------------------------------------
// Copyright (c) 2013 Stefan Pflueger.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//   Stefan Pflueger - initial API and implementation
//-------------------------------------------------------------------------------

#include <algorithm>
#include <stdexcept>

#include "DecayConfiguration.hpp"
#include "Core/PhysConst.hpp"

namespace HelicityFormalism {

DecayConfiguration::DecayConfiguration() {
}

DecayConfiguration::~DecayConfiguration() {
}

void DecayConfiguration::addCurrentDecayTreeToList() {
  concrete_decay_trees_.push_back(current_concrete_decay_tree_);
  current_concrete_decay_tree_.clear();
}

/*unsigned int DecayConfiguration::convertParticleIDToListIndex(
 unsigned int particle_id) const {
 ParticleStateIDComparator ps_comparator(particle_id);
 std::vector<ParticleStateInfo>::const_iterator found_particle = std::find_if(
 particles_.begin(), particles_.end(), ps_comparator);
 if (found_particle != particles_.end()) {
 return std::distance(particles_.begin(), found_particle);
 }
 else {
 throw std::runtime_error(
 "Could not find a particle with the correct ID within the list of particles!");
 }
 }

 std::vector<unsigned int> DecayConfiguration::convertParticleIDListToIndexList(
 const std::vector<unsigned int>& particle_id_list) const {
 std::vector<unsigned int> index_list;
 for (unsigned int i = 0; i < particle_id_list.size(); ++i) {
 index_list.push_back(convertParticleIDToListIndex(particle_id_list[i]));
 }
 return index_list;
 }*/

void DecayConfiguration::addDecayToCurrentDecayTree(
    const ParticleStateInfo& mother,
    const std::vector<ParticleStateInfo>& daughter_states,
    const boost::property_tree::ptree& decay_strength_info_and_phase) {

  // add particles to list if not already existent and get index
  unsigned int mother_state_index = addParticleToList(mother);

  DecayProductsInfo products;
  products.decay_strength_info_and_phase_ = decay_strength_info_and_phase;
  products.particle_indices_ = addParticlesToList(daughter_states);

  current_concrete_decay_tree_[mother_state_index].push_back(products);
}

std::vector<unsigned int> DecayConfiguration::addParticlesToList(
    const std::vector<ParticleStateInfo>& particle_list) {
  std::vector<unsigned int> index_list;
  for (unsigned int i = 0; i < particle_list.size(); ++i) {
    index_list.push_back(addParticleToList(particle_list[i]));
  }
  return index_list;
}

unsigned int DecayConfiguration::addParticleToList(ParticleStateInfo particle) {
  //ParticleStateIDComparator ps_comparator(particle_id);
  //std::vector<ParticleStateInfo>::const_iterator found_particle = std::find_if(
  //particles_.begin(), particles_.end(), ps_comparator);

  // first make sure the contents of the particle are all set (correctly)
  setRemainingParticleProperties(particle);

  unsigned int index(0);
  std::vector<ParticleStateInfo>::iterator found_particle = std::find(
      particles_.begin(), particles_.end(), particle);
  if (found_particle != particles_.end()) {
    index = std::distance(particles_.begin(), found_particle);
  }
  else {
    index = particles_.size();
    particles_.push_back(particle);
  }
  return index;
}

void DecayConfiguration::setRemainingParticleProperties(
    ParticleStateInfo& particle) const {

  PhysConst *physics_constants = PhysConst::instance();

  if (physics_constants->particleExists(particle.id_information_.name_)) {
    if (particle.id_information_.particle_id_
        != physics_constants->getId(particle.id_information_.name_)) {
      particle.id_information_.particle_id_ = physics_constants->getId(
          particle.id_information_.name_);
    }
    if (particle.spin_information_.J_
        != physics_constants->getJ(particle.id_information_.name_)) {
      particle.spin_information_.J_ = physics_constants->getJ(
          particle.id_information_.name_);
    }
  }
}

} /* namespace HelicityFormalism */
