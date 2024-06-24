/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * SimpleCone.cpp
 *
 * @brief algorithm calculates the sum of jets ET around each jet within
 * radius R, makes cut on leading sum ET
 *
 **********************************/

#include "SimpleCone.h"
#include "L1TopoSimulationUtils/Kinematics.h"
#include "L1TopoEvent/CompositeTOB.h"

#include "../IO/GenericTOBArray.h"
#include "../IO/Decision.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {
  SimpleCone::SimpleCone(const std::string & name,
			 unsigned int InputWidth,
			 unsigned int MaxRSqr,
			 unsigned int MinET,
			 const std::vector<unsigned int>& MinSumET) :
    m_name{name},
    m_InputWidth{InputWidth},
    m_MaxRSqr{MaxRSqr},
    m_MinET{MinET},
    m_MinSumET{MinSumET} {
  }

  StatusCode
  SimpleCone::run(const GlobalSim::GenericTOBArray& input,
      std::vector<GenericTOBArray>& output,
      Decision& decision) {
  
  

    auto numberOutputBits = m_MinSumET.size();
    output.clear();
    output.reserve(numberOutputBits);
    for (std::size_t i = 0; i != numberOutputBits; ++i) {
      std::stringstream ss;
      ss << "SimpleCone bit " << i;
      output.push_back(GenericTOBArray(ss.str()));
    }

    decision.setNBits(numberOutputBits);
    
    unsigned int leadingET = 0;

    auto end = std::min(input.begin() + m_InputWidth, input.end());
    for( GenericTOBArray::const_iterator itr0 = input.begin(); 
	 itr0 != end;
	 ++itr0) {
      
      if((*itr0)->Et() <= m_MinET ) continue; // E_T cut
      
      unsigned int tmp_SumET = (*itr0)->Et();
      
      for(GenericTOBArray::const_iterator itr1 = input.begin();
	  itr1 != end;
	  ++itr1) {
	
	if( itr1 == itr0 ) {
	  continue; // Avoid double counting of central jet
	}
	if((*itr1)->Et() <= m_MinET) continue; // E_T cut
	
	unsigned int deltaR2 = TSU::Kinematics::calcDeltaR2BW(*itr1, *itr0);
      
	if (deltaR2 > m_MaxRSqr) {
	  continue; // Exclude jets outside cone
	}
	
	tmp_SumET += (*itr1)->Et();
      }
    
      if (tmp_SumET > leadingET) {
	leadingET = tmp_SumET;
      }
    }
    
    for(unsigned int i=0; i< numberOutputBits; ++i) { 
      
      bool accept = leadingET > m_MinSumET[i];
      decision.setBit(i, accept);
      
      if(accept) {
	output[i].push_back(TCS::CompositeTOB(TCS::GenericTOB::createOnHeap(TCS::GenericTOB(leadingET,0,0))));
	m_ETPassByBit[i].push_back(leadingET);
      } else{ 
	m_ETFailByBit[i].push_back(leadingET);
      }
    }
  
    return StatusCode::SUCCESS;
  }

  const std::vector<double>& SimpleCone::ETPassByBit(std::size_t i) const {
    return m_ETPassByBit.at(i);
  }

  const std::vector<double>& SimpleCone::ETFailByBit(std::size_t i) const {
    return m_ETFailByBit.at(i);
  }

  std::string SimpleCone::toString() const {
    std::stringstream ss;
    ss <<  "SimpleCone. name: " << m_name << '\n'
       << " InputWidth: " << m_InputWidth
       << " MaxRSqr " << m_MaxRSqr
       << " MinET " << m_MinET
       << " MinSumET ["  << m_MinSumET.size() << "]: ";

    
    for (const auto& et : m_MinSumET) {
      ss << et << " ";
    }

    return ss.str();
  };

}
