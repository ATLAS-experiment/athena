# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# This package is a collection of 'duel-use' tools to calculate
# high-level flavor tagging discriminants. Because it should work both
# inside and outside Athena, nothing here can use the magnetic field,
# atlas geometry, or material maps, but neural networks etc are all
# fine.

# Declare the package name:
atlas_subdir( FlavorTagDiscriminants )

# External dependencies:
find_package( Boost )
find_package( lwtnn )
find_package( ROOT COMPONENTS Core MathCore )
find_package( onnxruntime )
find_package( nlohmann_json )

# source files, depend on the build
set(FTDSource
  Root/BTagJetAugmenter.cxx
  Root/BTagTrackIpAccessor.cxx
  Root/BTagAugmenterTool.cxx
  Root/BTagMuonAugmenter.cxx
  Root/BTagMuonAugmenterTool.cxx
  Root/DL2.cxx
  Root/DL2HighLevel.cxx
  Root/DL2Tool.cxx
  Root/DataPrepUtilities.cxx
  Root/OnnxUtil.cxx
  Root/OnnxOutput.cxx
  Root/GNN.cxx
  Root/GNNTool.cxx
  Root/GNNToolifiers.cxx
  Root/customGetter.cxx
  Root/FlipTagEnums.cxx
  Root/AssociationEnums.cxx
  Root/VRJetOverlapDecorator.cxx
  Root/VRJetOverlapDecoratorTool.cxx
  Root/HbbTag.cxx
  Root/HbbTagTool.cxx
  Root/HbbTagConfig.cxx
  Root/HbbGraphConfig.cxx
  Root/VRJetOverlapDecorator.cxx
  Root/FTagDataDependencyNames.cxx
  Root/TrackClassifier.cxx
  Root/StringUtils.cxx
)
if (NOT XAOD_STANDALONE)
  list(
    APPEND FTDSource
    Root/MultifoldGNN.cxx
    Root/MultifoldGNNTool.cxx
    )
endif()

# Build a shared library:
atlas_add_library( FlavorTagDiscriminants
  ${FTDSource}
  PUBLIC_HEADERS FlavorTagDiscriminants
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils nlohmann_json::nlohmann_json
  LINK_LIBRARIES ${Boost_LIBRARIES} ${LWTNN_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} 
    AsgTools
    AthContainers
    AthLinks
    JetInterface
    xAODBTagging
    xAODEventInfo
    xAODJet
    xAODTracking
    InDetTrackSystematicsToolsLib
    xAODMuon
    xAODEgamma
    ElectronPhotonSelectorToolsLib
  )


if (NOT XAOD_STANDALONE)
  atlas_add_component( FlavorTagDiscriminantsLib
    src/BTagDecoratorAlg.cxx
    src/JetTagDecoratorAlg.cxx
    src/BTagToJetLinkerAlg.cxx
    src/JetToBTagLinkerAlg.cxx
    src/BTagTrackLinkCopyAlg.cxx
    src/BTaggingBuilderAlg.cxx
    src/PoorMansIpAugmenterAlg.cxx
    src/JetTagConditionalDecoratorAlg.cxx
    src/TruthDecoratorHelpers.cxx
    src/TrackLeptonDecoratorAlg.cxx
    src/TruthParticleDecoratorAlg.cxx
    src/TrackTruthDecoratorAlg.cxx
    src/SoftElectronDecoratorAlg.cxx
    src/SoftElectronTruthDecoratorAlg.cxx
    src/FoldDecoratorAlg.cxx
    src/components/FlavorTagDiscriminants_entries.cxx
    LINK_LIBRARIES FlavorTagDiscriminants
    )
endif()

atlas_add_dictionary( FlavorTagDiscriminantsDict
   FlavorTagDiscriminants/FlavorTagDiscriminantsDict.h
   FlavorTagDiscriminants/selection.xml
   LINK_LIBRARIES FlavorTagDiscriminants )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# add onnx metadata executable
atlas_add_executable( get-onnx-model-info
                      util/get-onnx-model-info.cxx
                      INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} )

atlas_add_executable( get-onnx-metadata
                      util/get-onnx-metadata.cxx
                      INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} )
