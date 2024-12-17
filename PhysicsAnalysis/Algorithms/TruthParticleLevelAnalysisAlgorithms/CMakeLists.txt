# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina

atlas_subdir( TruthParticleLevelAnalysisAlgorithms )

find_package( ROOT COMPONENTS Core Physics )

atlas_add_library( TruthParticleLevelAnalysisAlgorithmsLib
  TruthParticleLevelAnalysisAlgorithms/*.h Root/*.cxx
  PUBLIC_HEADERS TruthParticleLevelAnalysisAlgorithms
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES AsgTools AsgMessagingLib xAODBase xAODEventInfo xAODJet xAODMissingET PATCoreLib AnaAlgorithmLib AsgDataHandlesLib MCTruthClassifierLib FourMomUtils SelectionHelpersLib
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODTruth RootCoreUtils )

atlas_add_dictionary( TruthParticleLevelAnalysisAlgorithmsDict
  TruthParticleLevelAnalysisAlgorithms/TruthParticleLevelAnalysisAlgorithmsDict.h
  TruthParticleLevelAnalysisAlgorithms/selection.xml
  LINK_LIBRARIES TruthParticleLevelAnalysisAlgorithmsLib )

if( NOT XAOD_STANDALONE )
  atlas_add_component( TruthParticleLevelAnalysisAlgorithms
    src/*.h src/*.cxx src/components/*.cxx
    LINK_LIBRARIES TruthParticleLevelAnalysisAlgorithmsLib )
endif()

atlas_install_python_modules( python/*.py )
