/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * EnergyThreshold_jXE.cxx
 * Based on L1TopoAlgorithm package 
**********************************/


#include "EnergyThreshold_jXE.h"

#include "../IO/jXETOBArray.h"
#include "../IO/Count.h"

#include <sstream>


namespace GlobalSim {
  EnergyThreshold_jXE::EnergyThreshold_jXE(const std::string& name,
					   unsigned int nbits,
					   unsigned int threshold):
    m_name{name},
    m_nbits{nbits},
    m_100MeVthreshold{threshold} // L1TopoSim uses L1Menu.thrValue100MeV()
  {

    if (m_nbits == 0) {
      throw std::runtime_error("EnergyThreshold_jXE m_nbits == 0");
    }
  }

  StatusCode EnergyThreshold_jXE::run(const jXETOBArray& jXEs,
					Count& count) {

    count.setNBits(m_nbits);

    int countVal{0}; 
    
    auto threshsq = m_100MeVthreshold * m_100MeVthreshold;

    for(const auto& jxe : jXEs) {
      m_inputET.push_back(jxe->Et());

      if(jxe->Et2() > threshsq) {
	++countVal;
	m_passingET.push_back(jxe->Et());
      }
      
    }

    count.setSizeCount(countVal);

    return StatusCode::SUCCESS;
  }

  std::string EnergyThreshold_jXE::toString() const {
    std::stringstream ss;
    ss << "EnergyThreshold_jXE. name: " << m_name << '\n'
       << " threshold " << m_100MeVthreshold
         << " numberOfBits " << m_nbits;
    return ss.str();
  }

  const std::vector<double>& EnergyThreshold_jXE::input_ET() const{
    return m_inputET;
  }
  
  const std::vector<double>& EnergyThreshold_jXE::passing_ET() const {
    return m_passingET;
  }

  const std::vector<double>& EnergyThreshold_jXE::counts() const {
    return m_counts;
  }
}
