# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( TrackingAnalysisAlgorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist Physics )

# Components in the package:
atlas_add_library( TrackingAnalysisAlgorithmsLib
   TrackingAnalysisAlgorithms/*.h Root/*.cxx
   PUBLIC_HEADERS TrackingAnalysisAlgorithms
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AnaAlgorithmLib EventBookkeeperToolsLib xAODTracking xAODTruth
   InDetSecVtxTruthMatchToolLib InDetTrackSystematicsToolsLib InDetTrackSelectionToolLib
   SelectionHelpersLib SystematicsHandlesLib PATCoreLib AsgDataHandlesLib AsgTools AthContainers
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_component( TrackingAnalysisAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES TrackingAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/* )
