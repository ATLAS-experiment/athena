# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# The name of the package:
atlas_subdir( MuonAnalysisAlgorithms )

atlas_add_library( MuonAnalysisAlgorithmsLib
   MuonAnalysisAlgorithms/*.h MuonAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS MuonAnalysisAlgorithms
   LINK_LIBRARIES xAODEventInfo xAODMuon SelectionHelpersLib
   SystematicsHandlesLib MuonAnalysisInterfacesLib IsolationSelectionLib
   AnaAlgorithmLib
   PRIVATE_LINK_LIBRARIES RootCoreUtils )

atlas_add_dictionary( MuonAnalysisAlgorithmsDict
   MuonAnalysisAlgorithms/MuonAnalysisAlgorithmsDict.h
   MuonAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES MuonAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( MuonAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel MuonAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )
