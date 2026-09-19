# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L1MuonInterface )

find_package( CLHEP )

atlas_add_library (L1MuonInterface
        L1MuonInterface/*.h src/*.cxx
        PUBLIC_HEADERS L1MuonInterface
	LINK_LIBRARIES AnaAlgorithmLib AthenaKernel AthenaMonitoringKernelLib 
      )




