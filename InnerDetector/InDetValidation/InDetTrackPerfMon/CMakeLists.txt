# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetTrackPerfMon )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread EG )
find_package( nlohmann_json )
## TODO - To be included in later MRs (if needed)
#find_package( XercesC )

# Component(s) in the package:
atlas_add_library( InDetTrackPerfMonLib
                   src/*.cxx src/plots/*.cxx
                   PUBLIC_HEADERS InDetTrackPerfMon
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaMonitoringLib CxxUtils FourMomUtils GaudiKernel InDetIdentifier TrkValHistUtils xAODBase xAODEventInfo xAODJet xAODTracking xAODTruth InDetPhysValMonitoringLib InDetTrackSelectionToolLib InDetTrackSystematicsToolsLib InDetRecToolInterfaces TrkTruthTrackInterfaces TrigDecisionToolLib AsgServicesLib 
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthContainers PATCoreAcceptLib PathResolver StoreGateLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkSurfaces TrkToolInterfaces TrkTrack TruthUtils nlohmann_json::nlohmann_json )

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

atlas_add_test ( hdefXmlToJson_test SCRIPT test/xml2Json.sh
                 POST_EXEC_SCRIPT nopost.sh )

atlas_add_test ( validateJson_test
                 SCRIPT test/validateJson.sh ${CMAKE_DATA_OUTPUT_DIRECTORY}/InDetTrackPerfMon
                 POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )
atlas_install_data( test/hdef.xml )
atlas_install_data( test/ref.json )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/test*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
atlas_install_data( scripts/hdefXmlToJson.xsl)
atlas_install_scripts( scripts/*.sh )
atlas_install_scripts( test/test*.sh )
