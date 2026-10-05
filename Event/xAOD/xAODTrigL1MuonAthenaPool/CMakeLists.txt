# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODTrigL1MuonAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODTrigL1MuonAthenaPoolCnv
   FILES xAODTrigL1Muon/SectorLogicCandDataContainer.h xAODTrigL1Muon/SectorLogicCandDataAuxContainer.h
         xAODTrigL1Muon/L1RPCCandDataContainer.h xAODTrigL1Muon/L1RPCCandDataAuxContainer.h
         xAODTrigL1Muon/L1TGCCandDataContainer.h xAODTrigL1Muon/L1TGCCandDataAuxContainer.h
         xAODTrigL1Muon/L1MDTCandDataContainer.h xAODTrigL1Muon/L1MDTCandDataAuxContainer.h
         xAODTrigL1Muon/L1NSWCandDataContainer.h xAODTrigL1Muon/L1NSWCandDataAuxContainer.h
   TYPES_WITH_NAMESPACE xAOD::SectorLogicCandDataContainer xAOD::SectorLogicCandDataAuxContainer
                        xAOD::L1RPCCandDataContainer xAOD::L1RPCCandDataAuxContainer
                        xAOD::L1TGCCandDataContainer xAOD::L1TGCCandDataAuxContainer
                        xAOD::L1MDTCandDataContainer xAOD::L1MDTCandDataAuxContainer
                        xAOD::L1NSWCandDataContainer xAOD::L1NSWCandDataAuxContainer
   CNV_PFX xAOD
   LINK_LIBRARIES AthContainers AthenaKernel AthenaPoolCnvSvcLib AthenaPoolUtilities xAODTrigL1Muon)
