# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( HyPERAnalysisAlgorithms )

find_package( onnxruntime )
find_package( ROOT REQUIRED COMPONENTS Core Physics )

atlas_add_library(
  HyPERAnalysisAlgorithmsLib
  HyPERAnalysisAlgorithms/*.h
  src/HyPERGraph.cxx
  src/HyPERTtbarLJetsParser.cxx
  src/HyPERTtbarDiLeptonParser.cxx
  src/HyPERTtbarAllHadronicParser.cxx
  src/HyPERTopoReco.cxx
  src/HyPERUtils.cxx
  src/RunHyPERAlg.cxx
  PUBLIC_HEADERS HyPERAnalysisAlgorithms
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
  LINK_LIBRARIES
    ${ROOT_LIBRARIES}
    ${ONNXRUNTIME_LIBRARIES}
    AthOnnxInterfaces
    AthOnnxUtilsLib
    AnaAlgorithmLib
    SystematicsHandlesLib
    SelectionHelpersLib
    PathResolver
    xAODEventInfo
    xAODJet
    xAODEgamma
    xAODMuon
    xAODMissingET
    xAODCore
    AthContainers
    FourMomUtils
)

atlas_add_component(
  HyPERAnalysisAlgorithms
  src/components/HyPERAnalysisAlgorithms_entries.cxx
  LINK_LIBRARIES HyPERAnalysisAlgorithmsLib
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_data( data/* )
