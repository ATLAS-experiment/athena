/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTrigger/versions/MuonRoIAuxContainer_v1.h"

namespace xAOD {

   MuonRoIAuxContainer_v1::MuonRoIAuxContainer_v1()
      : AuxContainerBase() {

      AUX_VARIABLE( eta );
      AUX_VARIABLE( phi );
      AUX_VARIABLE( roiWord );
      AUX_VARIABLE( thrName );
      AUX_VARIABLE( thrValue );
      AUX_VARIABLE( roiExtraWord );

   }

} // namespace xAOD
