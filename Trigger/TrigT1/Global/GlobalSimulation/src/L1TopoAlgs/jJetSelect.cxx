/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//
//  algorithm to create abbreviated jJet lists
//
#include "jJetSelect.h"

#include "../IO/jJetTOBArray.h"
#include "../IO/GenericTOBArray.h"

#include <sstream>

namespace GlobalSim {

  jJetSelect::jJetSelect(const std::string& name,
			 unsigned int InputWidth,
			 unsigned int MinET,
			 unsigned int MinEta,
			 unsigned int MaxEta):
    m_name{name},
    m_InputWidth{InputWidth},
    m_MinET{MinET},
    m_MinEta{MinEta},
    m_MaxEta{MaxEta}{
  }

  StatusCode
  jJetSelect::run(const jJetTOBArray& jets,
		  GenericTOBArray & output) const {
    
 
    
    // fill output array with GenericTOBs builds from jets

    std::size_t njets{0};
    for (const auto& jet : jets) {
      unsigned int Et = jet->Et();
      unsigned int eta = std::abs(jet-> eta());
      if(Et <= m_MinET or eta < m_MinEta or eta > m_MaxEta){
	continue;
      }
      
      output.push_back(TCS::GenericTOB(*jet));
      if (++njets > m_InputWidth) {break;}
    }
  
    return StatusCode::SUCCESS;
  }

  std::string jJetSelect::toString() const {
    std::stringstream ss;
    ss << "jJetSelect. name: " << m_name << '\n'
       << " maxJets: " << m_InputWidth
       << " minET " << m_MinET
       << " minEta " << m_MinEta
       << " maxEta " << m_MaxEta;

    return ss.str();
  }
}

