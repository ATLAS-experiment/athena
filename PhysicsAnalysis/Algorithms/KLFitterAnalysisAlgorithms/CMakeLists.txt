# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Oliver Majersky
# @author Baptiste Ravina

atlas_subdir( KLFitterAnalysisAlgorithms )

if(XAOD_ANALYSIS)
  # Find KLFitter package in atlasexternals,
  # source from https://www.github.com/KLFitter/KLFitter
  find_package(KLFitter)

  find_package( ROOT COMPONENTS Core Physics)

  atlas_add_library( KLFitterAnalysisAlgorithmsLib
    KLFitterAnalysisAlgorithms/*.h Root/*.cxx
    INCLUDE_DIRS ${KLFITTER_INCLUDE_DIRS}
    PUBLIC_HEADERS KLFitterAnalysisAlgorithms
    LINK_LIBRARIES ${KLFITTER_LIBRARIES}
    AnaAlgorithmLib FourMomUtils SystematicsHandlesLib SelectionHelpersLib xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET xAODCore
    FTagAnalysisInterfacesLib PathResolver )

  if(XAOD_STANDALONE)
    target_include_directories(KLFitterAnalysisAlgorithmsLib PUBLIC ${ROOT_INCLUDE_DIRS})
    target_link_libraries(KLFitterAnalysisAlgorithmsLib PUBLIC ${ROOT_LIBRARIES} AthContainers)
  endif()

  atlas_add_component(
    KLFitterAnalysisAlgorithms src/components/*.cxx
    LINK_LIBRARIES KLFitterAnalysisAlgorithmsLib
  )
  if(XAOD_STANDALONE)
    atlas_add_dictionary( KLFitterAnalysisAlgorithmsDict
      KLFitterAnalysisAlgorithms/KLFitterAnalysisAlgorithmsDict.h
      KLFitterAnalysisAlgorithms/selection.xml
      LINK_LIBRARIES KLFitterAnalysisAlgorithmsLib
      EXTRA_FILES Root/dict/*.cxx )
  else()
    atlas_add_dictionary( KLFitterAnalysisAlgorithmsDict
      KLFitterAnalysisAlgorithms/KLFitterAnalysisAlgorithmsDict.h
      KLFitterAnalysisAlgorithms/selection_ath.xml
      LINK_LIBRARIES KLFitterAnalysisAlgorithmsLib
      EXTRA_FILES Root/dict/*.cxx )
  endif()

  atlas_install_python_modules( python/*.py )

endif()
