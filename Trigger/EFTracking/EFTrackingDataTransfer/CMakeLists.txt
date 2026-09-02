# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( EFTrackingDataTransfer )

# Make use of the SYCL support in VecMem for building this package.
find_package( vecmem COMPONENTS LANGUAGE )

find_package(gRPC REQUIRED)
#find_package(Protobuf REQUIRED)
find_package(OpenSSL)

message(STATUS "Protobuf version: ${Protobuf_VERSION}")
message(STATUS "Protobuf includes: ${Protobuf_INCLUDE_DIRS}")

atlas_add_library(EFTrackingDataTransferLib
  src/*.cxx EFTrackingDataTransfer/*.pb.cc
  NO_PUBLIC_HEADERS
  LINK_LIBRARIES AnaAlgorithmLib EventInfo ByteStreamData ByteStreamCnvSvcBaseLib protobuf::libprotobuf gRPC::grpc++)

# Tests
atlas_add_test( TransferBenchmark_test
	SOURCES test/TransferBenchmark_test.cxx
	LINK_LIBRARIES TestTools ActsEventLib xAODInDetMeasurement
	POST_EXEC_SCRIPT noerror.sh)

# atlas_add_test( SyclTransferBenchmark_test 
# 	SOURCES test/SyclTransferBenchmark_test.sycl
# 	LINK_LIBRARIES TestTools ActsEventLib xAODInDetMeasurement AsgTools
# 	POST_EXEC_SCRIPT noerror.sh)

# tried that to generate c++ binding on the fly, failed
# 
# target_include_directories(EFTrackingDataTransferLib PRIVATE
#  ${CMAKE_CURRENT_BINARY_DIR})

# protobuf_generate(TARGET EFTrackingDataTransferLib
#   LANGUAGE cpp
#   PROTOS EFTrackingDataTransfer/Message.proto
#   APPEND_PATH EFTrackingDataTransfer
# )

# protobuf_generate(
#   TARGET EFTrackingDataTransferLib
#   LANGUAGE grpc
#   PROTOS EFTrackingDataTransfer/Message.proto
#   APPEND_PATH EFTrackingDataTransfer
#   GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
#   PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
# )

atlas_add_component(EFTrackingDataTransfer
        src/components/*.cxx
		LINK_LIBRARIES EFTrackingDataTransferLib )

atlas_add_executable( TestRPCService
                      test/TestRPCService.cxx
                      LINK_LIBRARIES protobuf::libprotobuf gRPC::grpc++ EFTrackingDataTransferLib )

atlas_install_python_modules(python/*)
