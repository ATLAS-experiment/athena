# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( AthExRemoteExec )

find_package( OpenSSL )
find_package( Protobuf )
find_package( gRPC )

# NOTE: If new proto schema are needed, they need to be compiled as well. Otherwise, proto schema from AthRemoteExecCore can be used
set( _generatedDir "${CMAKE_CURRENT_BINARY_DIR}/generated" )
set( _protoFiles "${CMAKE_CURRENT_SOURCE_DIR}/proto/athexremoteexec_demo.proto" )
file( MAKE_DIRECTORY "${_generatedDir}" )

protobuf_generate(
   LANGUAGE cpp
   OUT_VAR _generatedMessageSources
   PROTOS ${_protoFiles}
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_generatedDir}" )

atlas_add_library( AthExRemoteExecProtoObj
  ${_generatedMessageSources}
  ${_generatedServiceSources}
  INCLUDE_DIRS ${_generatedDir}
  LINK_LIBRARIES gRPC::grpc++ protobuf::libprotobuf
  NO_PUBLIC_HEADERS )

# FIXME: Intentional to hide torrent of warnings from generated protobuf code which does not follow ATLAS standard
target_compile_options( AthExRemoteExecProtoObj PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )

atlas_add_library( AthExProtocolLib
   src/DemoSchema.cxx
   NO_PUBLIC_HEADERS
   PRIVATE_INCLUDE_DIRS "${_generatedDir}"
   PRIVATE_LINK_LIBRARIES AthRemoteExecWireLib AthExRemoteExecProtoObj gRPC::grpc++
                          protobuf::libprotobuf )

atlas_add_component( AthExRemoteExec
   src/DemoIntsCodec.cxx
   src/DemoDoublesCodec.cxx
   src/DemoWordsCodec.cxx
   src/DemoClientAlgs.cxx
   src/SumAlg.cxx
   src/CountWordsAlg.cxx
   src/components/*.cxx
   LINK_LIBRARIES AthRemoteExecCoreLib AthExProtocolLib AthenaHiveEventLoopMgrLib
                  AthenaBaseComps AthenaKernel CxxUtils GaudiKernel SGTools StoreGateLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
