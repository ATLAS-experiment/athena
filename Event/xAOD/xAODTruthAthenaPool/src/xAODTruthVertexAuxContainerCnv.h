// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_H
#define XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolAuxContainerCnv.h"

// EDM include(s):
#include "xAODTruth/TruthVertexAuxContainer.h"
#include "xAODTruthVertexAuxContainerCnv_v2.h"

/// Base class for the converter
typedef T_AthenaPoolAuxContainerCnv< xAOD::TruthVertexAuxContainer,
                                     xAODTruthVertexAuxContainerCnv_v2 >
   xAODTruthVertexAuxContainerCnv;


#endif // XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_H
