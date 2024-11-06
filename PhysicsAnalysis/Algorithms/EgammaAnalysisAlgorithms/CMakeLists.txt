# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


# The name of the package:
atlas_subdir( EgammaAnalysisAlgorithms )

atlas_add_library( EgammaAnalysisAlgorithmsLib
   EgammaAnalysisAlgorithms/*.h EgammaAnalysisAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS EgammaAnalysisAlgorithms
   LINK_LIBRARIES xAODEgamma SelectionHelpersLib SystematicsHandlesLib
   IsolationSelectionLib AnaAlgorithmLib IsolationCorrectionsLib FourMomUtils
   EgammaAnalysisInterfacesLib egammaUtils CxxUtils )

atlas_add_dictionary( EgammaAnalysisAlgorithmsDict
   EgammaAnalysisAlgorithms/EgammaAnalysisAlgorithmsDict.h
   EgammaAnalysisAlgorithms/selection.xml
   LINK_LIBRARIES EgammaAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
   atlas_add_component( EgammaAnalysisAlgorithms
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES GaudiKernel EgammaAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
