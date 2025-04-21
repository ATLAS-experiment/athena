# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L0MuonInterface )

find_package( CLHEP )

atlas_add_library (L0MuonInterface
        L0MuonInterface/*.h src/*.cxx
        PUBLIC_HEADERS L0MuonInterface
	LINK_LIBRARIES AnaAlgorithmLib AthenaKernel AthenaMonitoringKernelLib
      )




