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
atlas_install_data( data/* )

atlas_add_test( TestCPRunElectrons
   SCRIPT CPRun.py --text-config EgammaAnalysisAlgorithms/test_configuration_Run2_Electrons.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 400 
)

atlas_add_test( TestCPRunPhotons
   SCRIPT CPRun.py --text-config EgammaAnalysisAlgorithms/test_configuration_Run2_Photons.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 400 
)