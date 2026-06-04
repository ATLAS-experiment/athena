# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODL0MuonCandAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODL0MuonCandAthenaPoolCnv
   FILES xAODL0MuonCand/RPCCandDataContainer.h xAODL0MuonCand/RPCCandDataAuxContainer.h 
         xAODL0MuonCand/TGCCandDataContainer.h xAODL0MuonCand/TGCCandDataAuxContainer.h
         xAODL0MuonCand/MDTCandDataContainer.h xAODL0MuonCand/MDTCandDataAuxContainer.h

   TYPES_WITH_NAMESPACE xAOD::RPCCandDataContainer xAOD::RPCCandDataAuxContainer 
                        xAOD::TGCCandDataContainer xAOD::TGCCandDataAuxContainer
                        xAOD::MDTCandDataContainer xAOD::MDTCandDataAuxContainer
   CNV_PFX xAOD
   LINK_LIBRARIES AthContainers AthenaKernel AthenaPoolCnvSvcLib AthenaPoolUtilities xAODL0MuonCand)

