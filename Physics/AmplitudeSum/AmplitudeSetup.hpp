//-------------------------------------------------------------------------------
// Copyright (c) 2013 Mathias Michel.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Mathias Michel - initial API and implementation
//		Peter Weidenkaff - adding parameters for flatte description
//-------------------------------------------------------------------------------
//! XML config parser for Amplitude Setup
/*!
 * @file AmplitudeSetup.hpp
 *\class AmplitudeSetup
 * This class is used to load an Amplitude configuration provided in an XML
 * file. As a Helper the struct Resonance is also provided, of which this class
 * stores a vector.
 */

#ifndef _AMPLITUDESETUP_HPP
#define _AMPLITUDESETUP_HPP

// Standard header files go here
#include <vector>
#include <string>
#include <memory>

// Boost header files go here
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/log/trivial.hpp>
using namespace boost::log;

struct Resonance
{
	bool m_enable;
	std::string m_name;
	std::string m_reference;
	double m_mass; //TODO: struct for borders?
	double m_mass_min;
	double m_mass_max;
	double m_mesonRadius;
	double m_width;
	double m_width_min;
	double m_width_max;
	double m_norm;

	double m_strength;
	double m_phase;
	unsigned int m_spin;
	unsigned int m_m;
	unsigned int m_n;

	unsigned int m_daugtherA; //TODO: better reference
	unsigned int m_daugtherB; //TODO: better reference
};
struct ResonanceFlatte : Resonance
{
	double m_coupling;
	double m_couplingHidden;
	std::string m_hiddenParticle1;
	std::string m_hiddenParticle2;

};
class AmplitudeSetup
{
private:
	std::string m_filePath;
	std::string m_file;          // ini filename
	std::vector<Resonance> m_resonances;          // resonances
	std::vector<ResonanceFlatte> m_resonancesFlatte;          // resonances

unsigned int nResEnabled;
unsigned int nRes;
public:
	AmplitudeSetup(const std::string &filename) : nRes(0),nResEnabled(0) { load(filename); };
	AmplitudeSetup(const AmplitudeSetup& other) : m_file(other.m_file), m_filePath(other.m_filePath),\
			m_resonances(other.m_resonances) , m_resonancesFlatte(other.m_resonancesFlatte),\
			nRes(other.nRes),nResEnabled(other.nResEnabled)
			{};
	void load(const std::string &filename);
	void save(const std::string &filename);
	~AmplitudeSetup(){};

	inline unsigned int getNResonances() {return m_resonances.size()+m_resonancesFlatte.size(); };
	inline const std::string & getFileName() const {return m_file;};
	inline const std::string & getFilePath() const {return m_filePath;};
	inline std::vector<Resonance> & getResonances() { return m_resonances; };
	inline std::vector<ResonanceFlatte> & getResonancesFlatte() { return m_resonancesFlatte; };
	inline unsigned int getNRes(){ return nRes; }
	inline unsigned int getNResEnabled(){ return nResEnabled; }

};
#endif
