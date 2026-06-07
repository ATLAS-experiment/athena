# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

atlas_subdir( EFTrackingDataTransfer )

# Make use of the SYCL support in VecMem for building this package.
find_package( vecmem COMPONENTS LANGUAGE )

find_package(Protobuf REQUIRED)
find_package(OpenSSL)
find_package(gRPC REQUIRED)
message(STATUS "gRPC_DIR = ${gRPC_DIR}")

atlas_add_library(EFTrackingDataTransferLib
  src/*.cxx src/*.cc
  NO_PUBLIC_HEADERS
  LINK_LIBRARIES AnaAlgorithmLib protobuf::libprotobuf gRPC::grpc++)

atlas_add_component(EFTrackingDataTransfer
        src/components/*.cxx
		LINK_LIBRARIES EFTrackingDataTransferLib )

atlas_add_executable( TestRPCService
                      test/TestRPCService.cxx
                      LINK_LIBRARIES protobuf::libprotobuf gRPC::grpc++ EFTrackingDataTransferLib )

atlas_install_python_modules(python/*)