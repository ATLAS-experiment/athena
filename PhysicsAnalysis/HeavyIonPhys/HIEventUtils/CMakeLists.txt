# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HIEventUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( HIEventUtilsLib
                   Root/*.cxx
                   PUBLIC_HEADERS HIEventUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AsgDataHandlesLib AsgTools CaloGeoHelpers InDetTrackSelectionToolLib PATCoreAcceptLib xAODForward xAODHIEvent xAODTracking xAODTrigL1Calo
                   PRIVATE_LINK_LIBRARIES AsgMessagingLib CxxUtils )

if (NOT XAOD_ANALYSIS)
  set( extra_libs CaloEvent )
  set( extra_src src/ExtractCaloGeoConstants.cxx )
endif()

if (NOT XAOD_STANDALONE)
  atlas_add_component( HIEventUtils
                       src/components/*.cxx src/HITowerWeightTool.cxx ${extra_src}
                       LINK_LIBRARIES AthenaBaseComps GaudiKernel HIEventUtilsLib Navigation ${extra_libs}
                       PRIVATE_LINK_LIBRARIES PathResolver)
endif ()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
