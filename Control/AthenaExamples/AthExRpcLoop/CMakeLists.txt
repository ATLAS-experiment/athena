# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExRpcLoop )

# External dependencies. OpenSSL first: gRPC's own CMake config declares
# OpenSSL::SSL in its link interface without finding it (same reason
# Control/AthTriton/AthTritonComps does this).
find_package( OpenSSL )
find_package( Protobuf )
find_package( gRPC )

if( NOT Protobuf_FOUND OR NOT gRPC_FOUND )
   message( STATUS "AthExRpcLoop: protobuf or gRPC not found, skipping the "
      "package" )
   return()
endif()

# Note what is *not* found here: ROOT, xAOD, or any detector. That is the
# claim this package makes, enforced by the linker rather than asserted in a
# comment -- the transport, the loop manager, the gate and the pack algorithm
# carry whatever a fragment puts on the boundary, and a serialisation library
# is one fragment's business rather than the framework's. A codec that streams
# dictionary-backed C++ objects is an ordinary Gaudi tool implementing
# src/IPayloadCodec.h, and belongs in a component library of its own; it is not
# here, and adding it must not add ROOT to this list.

# protoc cannot run bare in the build environment (libprotobuf/abseil are not in
# LD_LIBRARY_PATH yet). Override the imported target's location with a wrapper
# that runs it under atlas_build_run.sh. Pattern taken from
# Database/EventIndex/EventIndexProducer.
set( _customProtocScript
   "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/atlas_protoc.sh" )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/atlas_protoc.sh.in"
   "${_customProtocScript}" @ONLY
   FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
                    GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
set_target_properties( protobuf::protoc PROPERTIES
   IMPORTED_LOCATION "${_customProtocScript}" )
unset( _customProtocScript )

# Two schemas, and the difference between them is the whole design:
#
#   athrpc.proto         the envelope. A payload is bytes plus an encoding and
#                        a schema name. It enumerates no payload types and
#                        never will.
#   athexrpc_demo.proto  one fragment's payload. The framework does not import
#                        it, does not link its generated code into anything
#                        that is not src/demo/, and would not change if it were
#                        deleted.
set( _generatedDir "${CMAKE_CURRENT_BINARY_DIR}/generated" )
file( MAKE_DIRECTORY "${_generatedDir}" )

set( _protoFiles
   "${CMAKE_CURRENT_SOURCE_DIR}/proto/athrpc.proto"
   "${CMAKE_CURRENT_SOURCE_DIR}/proto/athexrpc_demo.proto" )

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
   PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/proto/athrpc.proto"
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_generatedDir}" )

# Generated code cannot be handed to atlas_add_library/atlas_add_executable:
# those expand their source arguments with file( GLOB ), which silently drops
# paths that do not exist yet -- so a clean build would link nothing while an
# incremental one would appear to work. Plain add_library( OBJECT ) understands
# the GENERATED property and gets the protoc ordering right for free.
#
# Generated protobuf/gRPC code also does not follow the ATLAS coding conventions
# (leading underscores, function-static tables), and cppcheck cannot resolve
# PROTOBUF_VERSION so it reports the gencode/runtime guard as an #error. Both
# analysers are silenced on the targets that see generated code, and nowhere
# else -- that boundary and the pimpl firewall are the same line.
function( athexrpc_add_proto_objects objName outDir )
   add_library( ${objName} OBJECT ${ARGN} )
   target_include_directories( ${objName} SYSTEM PUBLIC "${outDir}" )
   target_link_libraries( ${objName} PUBLIC gRPC::grpc++ protobuf::libprotobuf )
   set_target_properties( ${objName} PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      CXX_CPPCHECK "" )
   target_compile_options( ${objName} PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )
endfunction()

athexrpc_add_proto_objects( AthExRpcLoopProtoObj "${_generatedDir}"
   ${_generatedMessageSources} ${_generatedServiceSources} )

# The one library that owns all generated protobuf/gRPC code. LCG ships
# protobuf as a static library, so its symbols get embedded here; keeping every
# generated header out of the package's other sources is what keeps a second
# protobuf runtime from appearing in the process. See src/RpcWire.h.
#
# src/demo/DemoSchema.cxx is inside that boundary rather than in the component
# library for exactly that reason, and it is the one place the arrangement
# strains: a real fragment lives in its own package and would want to own its
# own generated code. Whether two statically-linked protobuf runtimes can share
# a process is the question to settle before promoting this out of
# AthenaExamples -- see also whether gRPC belongs in atlasexternals at all.
# Nothing else about the design depends on the answer: DemoSchema.h is a plain
# C++ interface, and moving what is behind it costs one CMake line.
atlas_add_library( AthExRpcLoopWireLib
   src/RpcWire.cxx src/RpcServer.cxx src/RpcClient.cxx src/RpcTiming.cxx
   src/demo/DemoSchema.cxx
   NO_PUBLIC_HEADERS
   PRIVATE_INCLUDE_DIRS "${_generatedDir}"
   PRIVATE_LINK_LIBRARIES AthExRpcLoopProtoObj gRPC::grpc++
                          protobuf::libprotobuf )

target_compile_options( AthExRpcLoopWireLib PRIVATE
   $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )
set_target_properties( AthExRpcLoopWireLib PROPERTIES CXX_CPPCHECK "" )

# The Gaudi components. None of these sources may include a generated header:
# they see the wire contract through RpcWire.h and RpcServer.h, and the
# demonstration fragment's payload through DemoSchema.h.
#
# A codec is a component too (see src/IPayloadCodec.h), so adding an encoding
# adds a tool rather than an entry in a table. The codecs that need ROOT, a
# detector or a device therefore belong in their own guarded component
# libraries, which is what keeps this one free of all three -- enforced by the
# linker rather than asserted in a comment, since nothing below finds ROOT.
atlas_add_component( AthExRpcLoop
   src/RpcHiveEventLoopMgr.cxx src/RpcBoundary.cxx
   src/RpcGateAlg.cxx src/RpcPackAlg.cxx src/RpcRequestAlg.cxx
   src/RpcPayloadCodecBase.cxx
   src/demo/DemoIntsCodec.cxx src/demo/DemoDoublesCodec.cxx
   src/demo/DemoClientAlgs.cxx
   src/demo/SumAlg.cxx src/demo/OffsetAlg.cxx src/demo/ScaleVectorAlg.cxx
   src/demo/RpcCondAlg.cxx src/demo/DelayAlg.cxx src/demo/FailAlg.cxx
   src/components/AthExRpcLoop_entries.cxx
   LINK_LIBRARIES AthExRpcLoopWireLib AthenaBaseComps AthenaKernel CxxUtils
                  GaudiKernel SGTools StoreGateLib )

# The clients, built the way a third party would build one: their own copy of
# the generated code, no AthExRpcLoop header, no Athena library. The only thing
# they share with the server is the two .proto files, which is exactly the
# claim the design makes. A second copy of the generated code in the build is
# fine -- the single-owner rule in src/RpcWire.h is about a single *process*,
# and these are separate executables.
set( _clientDir "${CMAKE_CURRENT_BINARY_DIR}/generated_client" )
file( MAKE_DIRECTORY "${_clientDir}" )

protobuf_generate(
   LANGUAGE cpp
   OUT_VAR _clientMessageSources
   PROTOS ${_protoFiles}
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_clientDir}" )

protobuf_generate(
   LANGUAGE grpc
   GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
   PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
   OUT_VAR _clientServiceSources
   PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/proto/athrpc.proto"
   IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
   PROTOC_OUT_DIR "${_clientDir}" )

athexrpc_add_proto_objects( AthExRpcLoopClientProtoObj "${_clientDir}"
   ${_clientMessageSources} ${_clientServiceSources} )

function( athexrpc_add_client name source )
   atlas_add_executable( ${name} ${source}
      INCLUDE_DIRS "${_clientDir}"
      LINK_LIBRARIES AthExRpcLoopClientProtoObj gRPC::grpc++
                     protobuf::libprotobuf )
   target_compile_options( ${name} PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-fplugin-arg-libchecker_gccplugins-checkers=none> )
   set_target_properties( ${name} PROPERTIES CXX_CPPCHECK "" )
endfunction()

athexrpc_add_client( rpc_menu_client test/rpc_menu_client.cxx )
athexrpc_add_client( rpc_load_client test/rpc_load_client.cxx )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh test/*.py )

# The generated code builds, links, and round-trips.
atlas_add_test( RpcWireRoundTrip
   SOURCES test/rpc_wire_roundtrip.cxx
   LINK_LIBRARIES AthExRpcLoopWireLib CxxUtils
   POST_EXEC_SCRIPT nopost.sh )

# The configuration derives a fragment's boundary from its payload's handles.
# A wrong derivation is not a configuration error but a server asking clients
# for the wrong thing, so each rule is pinned down -- including the cases where
# the right answer is to refuse.
atlas_add_test( RpcFragmentDerivation
   SCRIPT test_rpc_fragment_derivation.py
   POST_EXEC_SCRIPT nopost.sh )

# A boundary is declared by two halves of the configuration -- the payload's
# handles and the codec -- and they have to agree about what lands in the store.
# This corrupts one of them and requires the job to refuse to start.
atlas_add_test( RpcCodecMismatch
   SCRIPT test/test_rpc_codec_mismatch.py
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 300 )

# Everything except the socket: slots, gates, control flow, replies and every
# error path, with no client process at all.
atlas_add_test( RpcHiveLoopCanned
   SCRIPT test/test_rpc_hive_canned.sh
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 300 )

# A real server over a real socket, driven by a client that has only the
# schemas.
atlas_add_test( RpcHiveLoopMenuClient
   SCRIPT test/test_rpc_menu_client.sh
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 300 )

# Athena to Athena, with both ends built from the same declarations, checking
# arithmetic rather than status codes.
atlas_add_test( RpcAthenaClient
   SCRIPT test/test_rpc_athena_client.sh
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 300 )

# An overloaded server must refuse rather than absorb, and the refusal must
# reach the client as something it can act on.
atlas_add_test( RpcBackpressure
   SCRIPT test/test_rpc_backpressure.sh
   POST_EXEC_SCRIPT nopost.sh
   PRIVATE_WORKING_DIRECTORY
   PROPERTIES TIMEOUT 300 )
