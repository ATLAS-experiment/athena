# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Oliver Majersky
# @author Baptiste Ravina

atlas_subdir( KLFitterAnalysisAlgorithms )

if(XAOD_ANALYSIS)
  # Find KLFitter package in atlasexternals,
  # source from https://www.github.com/KLFitter/KLFitter
  find_package(KLFitter)

  find_package( ROOT COMPONENTS Core )

  # Generate a CINT dictionary source file:
  atlas_add_root_dictionary(KLFitterAnalysisAlgorithmsLib _cintDictSource
    ROOT_HEADERS Root/LinkDef.h
    EXTERNAL_PACKAGES ROOT )

  atlas_add_library( KLFitterAnalysisAlgorithmsLib
    KLFitterAnalysisAlgorithms/*.h Root/*.cxx ${_cintDictSource}
    PUBLIC_HEADERS KLFitterAnalysisAlgorithms
    INCLUDE_DIRS ${KLFITTER_INCLUDE_DIRS}
    PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${KLFITTER_LIBRARIES}
    AnaAlgorithmLib FourMomUtils SystematicsHandlesLib SelectionHelpersLib xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET
    FTagAnalysisInterfacesLib PathResolver
    PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData )

  atlas_add_dictionary( KLFitterAnalysisAlgorithmsDict
    KLFitterAnalysisAlgorithms/KLFitterAnalysisAlgorithmsDict.h
    KLFitterAnalysisAlgorithms/selection.xml
    LINK_LIBRARIES KLFitterAnalysisAlgorithmsLib
    EXTRA_FILES Root/dict/*.cxx )

  atlas_install_python_modules( python/*.py )

endif()
