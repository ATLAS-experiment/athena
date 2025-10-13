# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIClusterGeoWeights )

find_package( ROOT COMPONENTS Core Hist MathCore RIO )

atlas_add_library( HIClusterGeoWeightsLib
                   src/*.cxx
                   PUBLIC_HEADERS HIClusterGeoWeights
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib xAODCaloEvent xAODEventInfo xAODHIEvent xAODTracking )

atlas_add_component( HIClusterGeoWeights
                     src/components/*.cxx
                     LINK_LIBRARIES HIClusterGeoWeightsLib )

# Install files from the package:
atlas_add_executable( compareHIClusterGeoFiles
   util/compareHIClusterGeoFiles.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES})

atlas_add_executable( makeHIRunIndex
   util/makeHIRunIndex.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES})

atlas_add_executable( makeHIResponse
   util/makeHIResponse.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES})

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
