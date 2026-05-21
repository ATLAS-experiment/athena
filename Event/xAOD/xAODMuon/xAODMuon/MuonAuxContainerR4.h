/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUON_MUONAUXCONTAINER$R4_H
#define XAODMUON_MUONAUXCONTAINER$R4_H
 
// Local include(s):
#include "xAODMuon/versions/MuonAuxContainer_v6.h"
 
namespace xAOD {
    /// Definition of the current muon aux container used in the
    /// Phase-II software.
    using MuonAuxContainerR4 = MuonAuxContainer_v6;
}

// Set up a CLID and StoreGate inheritance for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MuonAuxContainerR4, 1217898686, 1 )


#endif // XAODMUON_MUONAUXCONTAINER_H
