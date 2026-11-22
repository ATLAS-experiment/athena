# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( AthRemoteExecGrpc )

find_package( OpenSSL )
find_package( Protobuf )
find_package( gRPC )
find_package( GTest )

set( _generatedDir "${CMAKE_CURRENT_BINARY_DIR}/generated" )
set( _protoFiles "${CMAKE_CURRENT_SOURCE_DIR}/proto/athremoteexec.proto" )
file( MAKE_DIRECTORY "${_generatedDir}" )

protobuf_generate(
   LANGUAGE cpp
   OUT_VAR _generatedMessageSources
   PROTOS ${_protoFiles}
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_generatedDir}" )

protobuf_generate(
   LANGUAGE grpc
   GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
   PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
   OUT_VAR _generatedServiceSources
   PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/proto/athremoteexec.proto"
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_generatedDir}" )

atlas_add_library( AthRemoteExecGrpcObj
  ${_generatedMessageSources}
  ${_generatedServiceSources}
  INCLUDE_DIRS ${_generatedDir}
  LINK_LIBRARIES gRPC::grpc++ protobuf::libprotobuf
  NO_PUBLIC_HEADERS )

# To make proto and gRPC headers available to other packages
target_include_directories( AthRemoteExecGrpcObj PUBLIC 
  $<BUILD_INTERFACE:${_generatedDir}>
  $<BUILD_INTERFACE:${CMAKE_INCLUDE_OUTPUT_DIRECTORY}>
  $<INSTALL_INTERFACE:include )
# FIXME: Intentional to hide torrent of warnings from generated protobuf code which does not follow ATLAS standard
target_compile_options( AthRemoteExecGrpcObj PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )

atlas_add_library( AthRemoteExecGrpcLib
   src/Protocol.cxx src/Server.cxx src/Client.cxx
   NO_PUBLIC_HEADERS
   PRIVATE_INCLUDE_DIRS "${_generatedDir}"
   PRIVATE_LINK_LIBRARIES AthRemoteExecGrpcObj AthRemoteExecWireLib )

atlas_add_test( RemoteExecGrpcInterfaceConversion
  SOURCES test/test_remote_exec_grpc_interface_conversion.cxx
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} AthRemoteExecGrpcLib AthRemoteExecGrpcObj AthRemoteExecWireLib
  POST_EXEC_SCRIPT noerror.sh )
# FIXME: Intentional to hide torrent of warnings from generated protobuf code which does not follow ATLAS standard
target_compile_options( AthRemoteExecGrpc_RemoteExecGrpcInterfaceConversion PRIVATE $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )
