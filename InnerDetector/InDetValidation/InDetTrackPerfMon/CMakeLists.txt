# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetTrackPerfMon )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread EG )
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( InDetTrackPerfMonLib
                   src/*.cxx src/plots/*.cxx
                   PUBLIC_HEADERS InDetTrackPerfMon
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib AthenaMonitoringLib CxxUtils FourMomUtils GaudiKernel InDetIdentifier TrkValHistUtils xAODBase xAODEventInfo xAODJet xAODTracking xAODTruth InDetPhysValMonitoringLib InDetTrackSelectionToolLib InDetTrackSystematicsToolsLib InDetRecToolInterfaces TrkTruthTrackInterfaces TrigDecisionToolLib AsgServicesLib 
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AthContainers PATCoreAcceptLib PathResolver StoreGateLib TrkEventPrimitives TrkExInterfaces TrkParameters TrkSurfaces TrkToolInterfaces TrigInDetAnalysisExampleLib TrkTrack TruthUtils nlohmann_json::nlohmann_json )

atlas_add_component( InDetTrackPerfMon
                     src/components/*.cxx
                     LINK_LIBRARIES InDetTrackPerfMonLib )

# build postprocessing program 
atlas_add_executable( postProcessIDTPMHistos util/postProcessIDTPMHistos.cxx
        INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
        LINK_LIBRARIES ${ROOT_LIBRARIES} InDetTrackPerfMonLib )

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
atlas_install_scripts( test/test*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( scripts/hdefXmlToJson.xsl )
atlas_install_scripts( scripts/*.sh )
atlas_install_scripts( test/test*.sh )
