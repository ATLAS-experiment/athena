# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( TrackingAnalysisAlgorithms )

# Components in the package:
atlas_add_library( TrackingAnalysisAlgorithmsLib
   TrackingAnalysisAlgorithms/*.h Root/*.cxx
   PUBLIC_HEADERS TrackingAnalysisAlgorithms
   LINK_LIBRARIES AnaAlgorithmLib EventBookkeeperToolsLib xAODTracking 
   InDetSecVtxTruthMatchToolLib InDetTrackSystematicsToolsLib InDetTrackSelectionToolLib
   SelectionHelpersLib AsgDataHandlesLib AsgTools AthContainers )

atlas_add_dictionary( TrackingAnalysisAlgorithmsDict
   TrackingAnalysisAlgorithms/TrackingAnalysisAlgorithmsDict.h
   TrackingAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES TrackingAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( TrackingAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel TrackingAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
