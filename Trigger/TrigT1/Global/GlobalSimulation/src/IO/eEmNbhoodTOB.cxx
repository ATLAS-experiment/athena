/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmNbhoodTOB.h"

namespace GlobalSim::IOBitwise {

  eEmNbhoodTOB::eEmNbhoodTOB(const xAOD::eFexEMRoI& eFexTOB,
			     const LArStripNeighborhood& nbhood) :eEmTOB(eFexTOB), m_neighbourhood(nbhood){}
  eEmNbhoodTOB::eEmNbhoodTOB(const IeEmTOB& IeEmTOB,
			     const LArStripNeighborhood& nbhood) : eEmTOB(IeEmTOB), m_neighbourhood(nbhood){}

  const LArStripNeighborhood& eEmNbhoodTOB::Neighbourhood() const {
    return m_neighbourhood;
  }

  std::string eEmNbhoodTOB::to_string() const {
    return IeEmNbhoodTOB::to_string();
  }

}
