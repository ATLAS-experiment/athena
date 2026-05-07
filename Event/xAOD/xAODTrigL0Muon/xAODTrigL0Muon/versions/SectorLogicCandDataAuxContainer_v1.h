/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATAAUXCONTAINER_V1_H
#define XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATAAUXCONTAINER_V1_H

// System include(s):
#include <cstdint>
#include <vector>
#include <string>

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"

namespace xAOD{

   class SectorLogicCandDataAuxContainer_v1 : public AuxContainerBase{

   public:

      // Default constuctor
      SectorLogicCandDataAuxContainer_v1();

   private:

      
      std::vector<uint32_t> candWord;          // First 32-bit information for candidate (pT val, charge, eta, phi)
      std::vector<uint32_t> candExtraWord;     // Remaining 32-bit information for candidate (pt threshold, MDT flags, coincidence flags, exotic trigger, TCID)
      std::vector<uint16_t> boardID;           // Information on what board the candidate is coming from
      std::vector<uint16_t> fiberID;           // Information on the link number between SL and MUCTPI
      std::vector<unsigned short> veto;        // Veto information needed for MUCTPI simulation
      std::vector<int> BCIDOffset;             // BCID offset needed for MUCTPI simulation

   }; // class SectorLogicCandDataAuxContainer_v1

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::SectorLogicCandDataAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATAAUXCONTAINER_V1_H
