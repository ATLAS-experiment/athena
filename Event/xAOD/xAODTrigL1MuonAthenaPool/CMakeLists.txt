# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODTrigL1MuonAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODTrigL1MuonAthenaPoolCnv
   FILES xAODTrigL1Muon/SectorLogicCandDataContainer.h xAODTrigL1Muon/SectorLogicCandDataAuxContainer.h
   TYPES_WITH_NAMESPACE xAOD::SectorLogicCandDataContainer xAOD::SectorLogicCandDataAuxContainer
   CNV_PFX xAOD
   LINK_LIBRARIES AthContainers AthenaKernel AthenaPoolCnvSvcLib AthenaPoolUtilities xAODTrigL1Muon)
