# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina

atlas_subdir( TruthPartonLevelAnalysisAlgorithms )

find_package( ROOT COMPONENTS Core MathCore GenVector )

atlas_add_library( TruthPartonLevelAnalysisAlgorithmsLib
  TruthPartonLevelAnalysisAlgorithms/*.h PartonHistory/*.h VectorHelpers/*.h Root/*.cxx
  PUBLIC_HEADERS TruthPartonLevelAnalysisAlgorithms PartonHistory VectorHelpers
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools AsgMessagingLib AthContainers xAODEventInfo xAODTruth AnaAlgorithmLib
  PRIVATE_LINK_LIBRARIES MCTruthClassifierLib )

atlas_add_component( TruthPartonLevelAnalysisAlgorithms
  src/components/*.cxx
  LINK_LIBRARIES TruthPartonLevelAnalysisAlgorithmsLib )

atlas_install_python_modules( python/*.py )
atlas_install_data( data/* )

atlas_add_test( TestCPRunParton
   SCRIPT CPRun.py --text-config TruthPartonLevelAnalysisAlgorithms/test_configuration_Run2_Parton.yaml -e 30 --input-list $ASG_TEST_FILE_MC
   POST_EXEC_SCRIPT noerror.sh
   PROPERTIES TIMEOUT 400
)
