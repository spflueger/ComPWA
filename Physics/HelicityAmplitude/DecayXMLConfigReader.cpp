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

#include "DecayXMLConfigReader.hpp"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

namespace HelicityFormalism {

using boost::property_tree::ptree;

DecayXMLConfigReader::DecayXMLConfigReader(
    DecayConfiguration &decay_configuration) :
    decay_configuration_(decay_configuration) {
}

DecayXMLConfigReader::~DecayXMLConfigReader() {
}

void DecayXMLConfigReader::readConfig(const std::string &filename) {
  // Create an empty property tree object

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(filename, pt_, boost::property_tree::xml_parser::trim_whitespace);
  pt_ = pt_.get_child("DecayTreeSetup");

  // add particle states to the template sets first
  // then we construct concrete states from those templates
  BOOST_FOREACH(ptree::value_type const& v, pt_.get_child("FinalState")){
  ParticleStateInfo ps = parseParticleStateBasics(v.second);
  template_particle_states_[ps.id_information_.id_] = ps;
}
  BOOST_FOREACH(ptree::value_type const& v, pt_.get_child("IntermediateStates")){
  ParticleStateInfo ps = parseParticleStateBasics(v.second);
  template_particle_states_[ps.id_information_.id_] = ps;
}

// now go through the decay tree info and construct them
  BOOST_FOREACH(ptree::value_type const& decay_tree, pt_.get_child("DecayTrees")){
  BOOST_FOREACH(ptree::value_type const& decay_node, decay_tree.second) {
    ParticleStateInfo mothers = parseParticleStateRemainders(decay_node.second.get_child("Mother").get_child("ParticleState"));
    std::vector<ParticleStateInfo> daughter_lists;
    BOOST_FOREACH(ptree::value_type const& daugthers, decay_node.second.get_child("Daughters")) {
      daughter_lists.push_back(parseParticleStateRemainders(daugthers.second));
    }

    ptree strength_phase;
    boost::optional<const ptree&> strength_phase_opt = decay_node.second.get_child_optional("StrengthPhase");
    if (strength_phase_opt.is_initialized()) {
      strength_phase = decay_node.second.get_child("StrengthPhase");
    }
    decay_configuration_.addDecayToCurrentDecayTree(mothers, daughter_lists, strength_phase);
  }
  decay_configuration_.addCurrentDecayTreeToList();
}
}

ParticleStateInfo DecayXMLConfigReader::parseParticleStateBasics(
    const boost::property_tree::ptree &pt) const {
  ParticleStateInfo ps;
  ps.id_information_.id_ = pt.get<unsigned int>("id");
  ps.id_information_.name_ = pt.get<std::string>("name");

  boost::optional<const ptree&> spin_info = pt.get_child_optional("SpinInfo");
  if (spin_info.is_initialized()) {
    ps.spin_information_.J_.SetSpin(
        pt.get_child("SpinInfo").get<unsigned int>("J"), 1);
  }
  else {
    // try to read it from a database
  }

  boost::optional<const ptree&> dynamical_info = pt.get_child_optional(
      "DynamicalInfo");
  if (dynamical_info.is_initialized()) {
    ps.dynamical_information_ = pt.get_child("DynamicalInfo");
  }
  else {
    // try to read it from a database
  }

  return ps;
}

ParticleStateInfo DecayXMLConfigReader::parseParticleStateRemainders(
    const boost::property_tree::ptree &pt) const {
  unsigned int id = pt.get<unsigned int>("id");

  // get the corresponding particle
  auto found_particle_state = template_particle_states_.find(id);
  if (found_particle_state != template_particle_states_.end()) {
    ParticleStateInfo ps(found_particle_state->second);
    ps.spin_information_.M_.SetSpin(pt.get<int>("M"), 1);

    return ps;
  }
  else {
    std::stringstream ss;
    ss << "Particle with id " << id
        << " not found in particle pool. Check your decay tree config file!";
    throw std::runtime_error(ss.str());
  }
}

/*std::vector<unsigned int> DecayXMLConfigReader::parseIDList(
 const boost::property_tree::ptree &pt) const {
 std::vector<unsigned int> ids;
 BOOST_FOREACH(ptree::value_type const &id_list_item, pt.get_child("IDList")){
 ids.push_back(id_list_item.second.get<unsigned int>(""));
 }
 return ids;
 }*/

}
/* namespace HelicityFormalism */
