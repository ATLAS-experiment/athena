//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODTruthParticleAuxContainerCnv_v2.h"
#include "xAODTruthVertexAuxContainerCnv_v2.h"

// EDM include(s):
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthVertexAuxContainer.h"
#include "xAODTruth/versions/TruthParticleAuxContainer_v2.h"
#include "xAODTruth/versions/TruthVertexAuxContainer_v2.h"

// Gaudi/Athena include(s).
#include "AthenaKernel/TPCnvFactory.h"

// Declare the T/P converter(s):
DECLARE_TPCNV_FACTORY(xAODTruthParticleAuxContainerCnv_v2,
                      xAOD::TruthParticleAuxContainer,
                      xAOD::TruthParticleAuxContainer_v2,
                      Athena::TPCnvVers::Old)
DECLARE_TPCNV_FACTORY(xAODTruthVertexAuxContainerCnv_v2,
                      xAOD::TruthVertexAuxContainer,
                      xAOD::TruthVertexAuxContainer_v2,
                      Athena::TPCnvVers::Old)
