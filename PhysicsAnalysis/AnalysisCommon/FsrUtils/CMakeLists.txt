# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FsrUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core Physics RIO )

# Libraries in the package:
atlas_add_library( FsrUtilsLib
   FsrUtils/*.h Root/*.cxx
   PUBLIC_HEADERS FsrUtils
   LINK_LIBRARIES AsgTools PATInterfaces xAODEgamma xAODMuon
   PRIVATE_LINK_LIBRARIES AthContainers EgammaAnalysisInterfacesLib IsolationCorrectionsLib
   IsolationSelectionLib xAODRootAccess EgammaAnalysisAlgorithmsLib)

if( NOT XAOD_STANDALONE )
   atlas_add_component( FsrUtils
      src/*.h src/*.cxx src/components/*.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel xAODMuon xAODEgamma
      FsrUtilsLib )
endif()

# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( FsrPhotonToolTester
      util/FsrPhotonToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} ElectronPhotonFourMomentumCorrectionLib xAODBase
      xAODBase xAODCore xAODEgamma xAODMuon xAODRootAccess FsrUtilsLib )
endif()
