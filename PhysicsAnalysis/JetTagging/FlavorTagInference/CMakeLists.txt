# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# This package is a collection of 'duel-use' tools to calculate
# high-level flavor tagging discriminants. Because it should work both
# inside and outside Athena, nothing here can use the magnetic field,
# atlas geometry, or material maps, but neural networks etc are all
# fine.

# Declare the package name:
atlas_subdir( FlavorTagInference )

# External dependencies:
find_package( lwtnn )
find_package( ROOT COMPONENTS Core MathCore )
find_package( onnxruntime )
find_package( nlohmann_json )

set(TRITON_LIBRARIES "")

if (NOT XAOD_ANALYSIS)
  find_package(gRPC CONFIG REQUIRED)
  find_package(TritonCommon CONFIG REQUIRED)
  find_package(TritonClient CONFIG REQUIRED COMPONENTS TritonClient::triton-client)

  set(TRITON_LIBRARIES TritonClient::grpcclient_static gRPC::grpc++)
endif()

# source files, depend on the build
set(FTISource
  Root/BTagTrackIpAccessor.cxx
  Root/ConstituentsLoader.cxx
  Root/CustomGetterUtils.cxx
  Root/DataPrepUtilities.cxx
  Root/ElectronsLoader.cxx
  Root/FlipTagEnums.cxx
  Root/FlowElementsLoader.cxx
  Root/FTagDataDependencyNames.cxx
  Root/GNN.cxx
  Root/GNNTool.cxx
  Root/GNNOptions.cxx
  Root/GNNToolifiers.cxx
  Root/SaltModel.cxx
  Root/SaltModelOutput.cxx
  Root/NNSharingSvc.cxx
  Root/StringUtils.cxx
  Root/TracksLoader.cxx
  Root/HitsLoader.cxx
  Root/SaltModelGraphConfig.cxx
  Root/GNNDataLoader.cxx
)

if (NOT XAOD_STANDALONE)
  list(
    APPEND FTISource
    Root/MultifoldGNN.cxx
    Root/MultifoldGNNTool.cxx
    )
endif()

if (NOT XAOD_ANALYSIS)
  list(
    APPEND FTISource
    Root/SaltModelTriton.cxx
    )
endif()


# Build a shared library:
atlas_add_library( FlavorTagInference
  ${FTISource}
  PUBLIC_HEADERS FlavorTagInference
  PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS}
  PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver CxxUtils nlohmann_json::nlohmann_json
  LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} ${LWTNN_LIBRARIES} ${TRITON_LIBRARIES}
    AsgTools
    AsgServicesLib
    AthContainers
    AthLinks
    JetInterface
    xAODEventInfo
    xAODJet
    xAODTracking
    xAODBTagging
    xAODEgamma
    xAODMuon
  )


if (NOT XAOD_STANDALONE)
  atlas_add_component( FlavorTagInferenceLib
    src/JetTagDecoratorAlg.cxx
    src/JetTagConditionalDecoratorAlg.cxx
    src/FoldDecoratorAlg.cxx
    src/components/FlavorTagInference_entries.cxx
    LINK_LIBRARIES FlavorTagInference
    )

  # Suppress spurious warnings seen in the LTO build originating in
  # the boost parser code.
  set_target_properties( FlavorTagInference
    PROPERTIES LINK_FLAGS " -Wno-maybe-uninitialized "  )

endif()

atlas_add_dictionary( FlavorTagInferenceDict
   FlavorTagInference/FlavorTagInferenceDict.h
   FlavorTagInference/selection.xml
   LINK_LIBRARIES FlavorTagInference )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
