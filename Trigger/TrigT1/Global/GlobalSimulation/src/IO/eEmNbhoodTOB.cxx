/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmNbhoodTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  eEmNbhoodTOB::eEmNbhoodTOB(const xAOD::eFexEMRoI& eFexTOB,
			     const LArStripNeighborhood& nbhood) :
    eEmTOB(eFexTOB), m_neighbourhood(nbhood){}

  eEmNbhoodTOB::eEmNbhoodTOB(const eEmTOB& tob,
			     const LArStripNeighborhood& nbhood) :
    eEmTOB(tob), m_neighbourhood(nbhood){}

  const LArStripNeighborhood& eEmNbhoodTOB::Neighbourhood() const {
    return m_neighbourhood;
  }

  std::string eEmNbhoodTOB::to_string() const {
    std::stringstream ss;
    
    ss << '\n'
       << IeEmTOB::to_string()
       << "\n"
       << "Neighbourhood content:\n"
       << Neighbourhood().to_string();
    return ss.str();
  }

}
