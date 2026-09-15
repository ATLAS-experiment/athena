atlas_subdir( HyPERAlgorithms )

find_package( onnxruntime )
find_package( ROOT REQUIRED COMPONENTS Core Physics )

atlas_add_library(
  HyPERAlgorithmsLib
  HyPERAlgorithms/*.h
  src/HyPERGraph.cxx
  src/HyPERTtbarLJetsParser.cxx
  src/HyPERTtbarDiLeptonParser.cxx
  src/HyPERTtbarAllHadronicParser.cxx
  src/HyPERTopoReco.cxx
  src/HyPERUtils.cxx
  src/RunHyPERAlg.cxx
  PUBLIC_HEADERS HyPERAlgorithms
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
  HyPERAlgorithms
  src/components/HyPERAlgorithms_entries.cxx
  LINK_LIBRARIES HyPERAlgorithmsLib
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_data( data/* )
