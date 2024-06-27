# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare package version:
set( RingerSelectorTools_version "\"RingerSelectorTools-00-00-18\"" )

# Declare the package name:
atlas_subdir( RingerSelectorTools )

# External dependencies:

find_package( ROOT COMPONENTS Core Gpad Tree MathCore Hist RIO pthread )
find_package( onnxruntime )



atlas_add_library( RingerSelectorToolsLib
                   Root/*.cxx
                   Root/procedures/*.cxx
                   Root/tools/*.cxx
                   Root/tools/onnx/*.cxx
                   PUBLIC_HEADERS RingerSelectorTools
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AsgTools CxxUtils xAODCaloRings xAODEgamma xAODTracking 
                   PATCoreLib EgammaAnalysisInterfacesLib xAODTrigRinger xAODTrigCalo xAODTrigEgamma
                   PRIVATE_LINK_LIBRARIES xAODBase PathResolver TrkTrackSummary AthOnnxInterfaces AthenaMonitoringKernelLib)

if( NOT XAOD_STANDALONE )
  atlas_add_component( RingerSelectorTools
                       src/components/*.cxx
                       INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
                       LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} ${ROOT_LIBRARIES} AthenaKernel AthContainers GaudiKernel TrkTrackSummary RingerSelectorToolsLib AthOnnxInterfaces)
endif()



# Install files from the package:
if( XAOD_STANDALONE )
atlas_add_executable( dumpRings
   util/dumpRings.cxx
   LINK_LIBRARIES RingerSelectorToolsLib ${ROOT_LIBRARIES} )

atlas_add_executable( ringsHist
   util/ringsHist.cxx
   LINK_LIBRARIES RingerSelectorToolsLib ${ROOT_LIBRARIES} )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if( XAOD_STANDALONE )
message( STATUS "Using tag ${RingerSelectorTools_version} for all written RingerSelectorTools data." )
endif()
remove_definitions( -DRINGER_SELECTOR_PACKAGE_VERSION )
add_definitions( -DRINGER_SELECTOR_PACKAGE_VERSION=${RingerSelectorTools_version} )

