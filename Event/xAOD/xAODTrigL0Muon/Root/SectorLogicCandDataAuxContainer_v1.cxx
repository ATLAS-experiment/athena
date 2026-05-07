/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigL0Muon/versions/SectorLogicCandDataAuxContainer_v1.h"

namespace xAOD {

   SectorLogicCandDataAuxContainer_v1::SectorLogicCandDataAuxContainer_v1()
      : AuxContainerBase() {

      AUX_VARIABLE( candWord );           // First 32-bit information for candidate (pT val, charge, eta, phi)
      AUX_VARIABLE( candExtraWord );      // Remaining 32-bit information for candidate (pt threshold, MDT flags, coincidence flags, exotic trigger, TCID)
      AUX_VARIABLE( boardID );            // Information on what board the candidate is coming from
      AUX_VARIABLE( fiberID );            // Information on the link number between SL and MUCTPI
      AUX_VARIABLE( veto );               // Veto information needed for MUCTPI simulation
      AUX_VARIABLE( BCIDOffset );         // BCID offset needed for MUCTPI simulation
   }

} // namespace xAOD
