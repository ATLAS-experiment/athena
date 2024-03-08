# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ByteStreamCnvSvcBase )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat eformat_write )

# Component(s) in the package:
atlas_add_library( ByteStreamCnvSvcBaseLib
                   src/*.cxx
                   PUBLIC_HEADERS ByteStreamCnvSvcBase
                   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel ByteStreamData GaudiKernel StoreGateLib
                   PRIVATE_LINK_LIBRARIES SGTools TestTools )

atlas_add_component( ByteStreamCnvSvcBase
                     src/components/*.cxx
                     LINK_LIBRARIES ByteStreamCnvSvcBaseLib )

# Tests in the package:
atlas_add_test( ROBDataProviderSvcMT
                SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_ROBDataProviderSvcMT.py
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT noerror.sh )
