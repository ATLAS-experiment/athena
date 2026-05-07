# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODTrigL0MuonAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODTrigL0MuonAthenaPoolCnv
   FILES xAODTrigL0Muon/SectorLogicCandDataContainer.h xAODTrigL0Muon/SectorLogicCandDataAuxContainer.h
   TYPES_WITH_NAMESPACE xAOD::SectorLogicCandDataContainer xAOD::SectorLogicCandDataAuxContainer
   CNV_PFX xAOD
   LINK_LIBRARIES AthContainers AthenaKernel AthenaPoolCnvSvcLib AthenaPoolUtilities xAODTrigL0Muon)
