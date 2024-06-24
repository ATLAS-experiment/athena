/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//
//  Algorithm to select the abbreviated list of eEMs , no order is applied
//
#include "eEmSelect.h"

#include "L1TopoEvent/GenericTOB.h"
#include "../IO/eEmTOBArray.h"
#include "../IO/GenericTOBArray.h"
#include "isocut.h"

#include <sstream>

namespace GlobalSim {
  eEmSelect::eEmSelect(const std::string& name,
		       unsigned int InputWidth,
		       unsigned int MinET,
		       unsigned int REtaMin,
		       unsigned int RHadMin,
		       unsigned int WsTotMin):

    m_name{name},
    m_InputWidth{InputWidth},
    m_MinET{MinET},
    m_REtaMin{REtaMin},
    m_RHadMin{RHadMin},
    m_WsTotMin{WsTotMin} {
  }
		       
  eEmSelect::~eEmSelect() {}

  StatusCode eEmSelect::run(const eEmTOBArray& eems,
			    GenericTOBArray & output) const {
    
    std::size_t neems{0};
    for (const auto& eem : eems) {
      if (eem->Et() <= m_MinET) {continue;}
      if (!isocut(m_REtaMin, eem-> Reta())) {continue;}
      if (!isocut(m_RHadMin, eem-> Rhad())) {continue;}
      if (!isocut(m_WsTotMin, eem-> Wstot())) {continue;}

      const TCS::GenericTOB gtob(*eem);
      output.push_back(gtob);
      if (++neems > m_InputWidth) {break;}
    }
    
    return StatusCode::SUCCESS;
  }

   std::string eEmSelect::toString() const {
    std::stringstream ss;
    ss << "eEmSelect. alg_name: " << m_name << '\n'
       << " max objects: " << m_InputWidth
       << " minET " << m_MinET
       << " REtaMin " << m_REtaMin
       << " RHadMin " << m_RHadMin
       << " WsTotMin " << m_WsTotMin;

    return ss.str();
  }
}

