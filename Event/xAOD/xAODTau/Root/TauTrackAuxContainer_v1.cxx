/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODTau/versions/TauTrackAuxContainer_v1.h"

namespace xAOD {

TauTrackAuxContainer_v1::TauTrackAuxContainer_v1()
   : AuxContainerBase() {

   AUX_VARIABLE( pt );
   AUX_VARIABLE( eta );
   AUX_VARIABLE( phi );

   AUX_VARIABLE( flagSet );

   AUX_VARIABLE( bdtScores );

   AUX_VARIABLE( trackLinks );

}
   
} // namespace xAOD
