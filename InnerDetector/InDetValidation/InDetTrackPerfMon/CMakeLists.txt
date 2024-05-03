# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetTrackPerfMon )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread EG )
find_package( Boost COMPONENTS unit_test_framework )
## TODO - To be included in later MRs (if needed)
#find_package( XercesC )
#find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( InDetTrackPerfMonLib
                   src/*.cxx
                   PUBLIC_HEADERS InDetTrackPerfMon
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaMonitoringLib CxxUtils GaudiKernel InDetIdentifier xAODBase xAODEventInfo xAODJet xAODTracking xAODTruth InDetTrackSystematicsToolsLib InDetRecToolInterfaces TrigDecisionToolLib AsgServicesLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthContainers PATCoreAcceptLib PathResolver StoreGateLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkSurfaces TrkToolInterfaces TrkTrack )

## TODO - To be included in later MRs (if needed)
#atlas_add_library( InDetTrackPerfMonLib
#                   src/*.cxx
#                   PUBLIC_HEADERS InDetTrackPerfMon
#                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS}
#                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaMonitoringLib AtlasDetDescr BeamSpotConditionsData CxxUtils GaudiKernel InDetIdentifier InDetTrackSelectionToolLib InDetTruthVertexValidationLib MCTruthClassifierLib TrkValHistUtils TRT_ReadoutGeometry xAODBase xAODEventInfo xAODJet xAODTracking xAODTruth InDetTrackSystematicsToolsLib InDetRecToolInterfaces TrigDecisionToolLib AsgServicesLib
#                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${XERCESC_LIBRARIES} AsgAnalysisInterfaces AsgTools AthContainers EventPrimitives GeoPrimitives InDetPrepRawData InDetRIO_OnTrack PATCoreAcceptLib PathResolver StoreGateLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkSurfaces TrkToolInterfaces TrkTrack nlohmann_json::nlohmann_json )

atlas_add_component( InDetTrackPerfMon
                     src/components/*.cxx
                     LINK_LIBRARIES InDetTrackPerfMonLib )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/test*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/test*.sh )
