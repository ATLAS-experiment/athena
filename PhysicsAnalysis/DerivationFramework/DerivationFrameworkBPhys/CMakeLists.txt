# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkBPhys )

# External dependencies:
find_package( Boost )
find_package( CLHEP )
find_package( ROOT COMPONENTS Core MathCore )
find_package( HepPDT )

atlas_add_library( DerivationFrameworkBPhysLib
  DerivationFrameworkBPhys/*.h src/*.cxx
  PUBLIC_HEADERS DerivationFrameworkBPhys
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} ${HEPPDT_LIBRARIES} ${ROOT_LIBRARIES}
  AthContainers AthLinks AthenaBaseComps AthenaKernel AthenaPoolUtilities BPhysToolsLib
  CaloEvent CxxUtils DerivationFrameworkInterfaces EventKernel EventPrimitives GeoPrimitives
  ITrackToVertex InDetConversionFinderToolsLib InDetTrackSelectionToolLib InDetV0FinderLib JpsiUpsilonToolsLib
  MuonAnalysisInterfacesLib RecoToolInterfaces StoreGateLib TrackToCaloLib TrackVertexAssociationToolLib
  TrigDecisionToolLib TrkCaloExtension TrkEventPrimitives TrkExInterfaces TrkParameters TrkSurfaces
  TrkToolInterfaces TrkV0FitterLib TrkVKalVrtFitterLib TrkVertexAnalysisUtilsLib TrkVertexFitterInterfaces
  TrkVertexSeedFinderUtilsLib xAODBPhysLib xAODBase xAODCore xAODEventInfo xAODMetaData xAODMuon
  xAODPrimitives xAODTracking xAODTruth )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkBPhys
  src/components/*.cxx
  LINK_LIBRARIES GaudiKernel DerivationFrameworkBPhysLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})

