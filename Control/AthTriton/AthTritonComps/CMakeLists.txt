# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthTritonComps )

# External dependencies.
find_package(gRPC CONFIG REQUIRED)
find_package(TritonCommon CONFIG REQUIRED)
find_package(TritonClient CONFIG REQUIRED COMPONENTS TritonClient::triton-client AthTritonInterfaces)

atlas_add_library( AthTritonCompsLib
   src/*.cxx
   PUBLIC_HEADERS AthTritonComps
   LINK_LIBRARIES ${PROTOBUF_LIBRARIES} TritonClient::grpcclient_static gRPC::grpc++
   AthenaBaseComps AthTritonInterfaces
)

atlas_add_dictionary( AthTritonCompsDict
    AthTritonComps/AthTritonCompsDict.h
    AthTritonComps/selection.xml
    LINK_LIBRARIES AthTritonCompsLib 
)

if (NOT XAOD_STANDALONE)
   atlas_add_component( AthTritonComps
   src/components/*.cxx
   LINK_LIBRARIES AthTritonCompsLib
)
endif()

# install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
