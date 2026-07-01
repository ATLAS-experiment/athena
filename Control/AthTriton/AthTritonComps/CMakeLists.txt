# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthTritonComps )

# External dependencies.
find_package(OpenSSL)
find_package(gRPC)
find_package(TritonCommon)
find_package(TritonClient)

atlas_add_library( AthTritonCompsLib
   AthTritonComps/*.h src/*.cxx
   PUBLIC_HEADERS AthTritonComps
   LINK_LIBRARIES AthenaBaseComps AthOnnxInterfaces
   PRIVATE_LINK_LIBRARIES TritonClient::grpcclient_static gRPC::grpc++
)

atlas_add_dictionary( AthTritonCompsDict
    AthTritonComps/AthTritonCompsDict.h
    AthTritonComps/selection.xml
    LINK_LIBRARIES AthTritonCompsLib
)

atlas_add_component( AthTritonComps
   src/components/*.cxx
   LINK_LIBRARIES AthTritonCompsLib
)

# install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
