/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTTWINMEASUREMENT_H
#define XAODMUONPREPDATA_MDTTWINMEASUREMENT_H

#include "xAODMuonPrepData/versions/MdtTwinDriftCircle_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the MdtDriftCircle
typedef MdtTwinDriftCircle_v1 MdtTwinDriftCircle;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MdtTwinDriftCircle , 71190612 , 1 )

#endif  // XAODMUONRDO_NRPCRDO_H
