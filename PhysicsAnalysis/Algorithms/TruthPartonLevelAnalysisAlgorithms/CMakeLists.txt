# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina

atlas_subdir( TruthPartonLevelAnalysisAlgorithms )

find_package( ROOT COMPONENTS Core Physics )

atlas_add_library( TruthPartonLevelAnalysisAlgorithmsLib
  TruthPartonLevelAnalysisAlgorithms/*.h PartonHistory/*.h VectorHelpers/*.h Root/*.cxx
  PUBLIC_HEADERS TruthPartonLevelAnalysisAlgorithms PartonHistory VectorHelpers
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES AsgMessagingLib xAODBase xAODEventInfo PATCoreLib AnaAlgorithmLib AsgDataHandlesLib MCTruthClassifierLib FourMomUtils
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODTruth RootCoreUtils )

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
