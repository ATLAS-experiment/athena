# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SCT_RawDataByteStreamCnv )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

atlas_add_library( SCT_RawDataByteStreamCnvLib
                   SCT_RawDataByteStreamCnv/*.h
                   INTERFACE
                   PUBLIC_HEADERS SCT_RawDataByteStreamCnv
                   LINK_LIBRARIES GaudiKernel InDetRawData ByteStreamCnvSvcBaseLib ByteStreamCnvSvcLib ByteStreamData InDetByteStreamErrors )

# Component(s) in the package:
atlas_add_component( SCT_RawDataByteStreamCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps ByteStreamCnvSvcBaseLib ByteStreamData EventContainers GaudiKernel IRegionSelector Identifier InDetIdentifier InDetRawData InDetReadoutGeometry SCT_CablingLib SCT_ConditionsData SCT_ConditionsToolsLib SCT_RawDataByteStreamCnvLib SCT_ReadoutGeometry StoreGateLib TrigSteeringEvent xAODEventInfo )

# Run tests:
atlas_add_test( TestSCTDecodeNewConf
                SCRIPT python -m SCT_RawDataByteStreamCnv.testSCTDecodeNewConf
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 600 )
atlas_add_test( TestSCTEncodeNewConf
                SCRIPT python -m SCT_RawDataByteStreamCnv.testSCTEncodeNewConf
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 600 )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
