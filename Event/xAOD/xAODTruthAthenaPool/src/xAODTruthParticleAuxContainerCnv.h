// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_H
#define XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolAuxContainerCnv.h"

// EDM include(s):
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruthParticleAuxContainerCnv_v1.h"

/// Base class for the converter
typedef T_AthenaPoolAuxContainerCnv< xAOD::TruthParticleAuxContainer,
                                     xAODTruthParticleAuxContainerCnv_v1 >
   xAODTruthParticleAuxContainerCnv;


#endif // XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_H
