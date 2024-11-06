# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( TauAnalysisAlgorithms )

atlas_add_library( TauAnalysisAlgorithmsLib
   TauAnalysisAlgorithms/*.h TauAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS TauAnalysisAlgorithms
   LINK_LIBRARIES xAODTau SelectionHelpersLib DiTauMassToolsLib
   SystematicsHandlesLib
   AnaAlgorithmLib TauAnalysisToolsLib )

atlas_add_dictionary( TauAnalysisAlgorithmsDict
   TauAnalysisAlgorithms/TauAnalysisAlgorithmsDict.h
   TauAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES TauAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( TauAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel TauAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )

atlas_install_data( data/*.conf )
