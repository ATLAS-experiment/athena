# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileByteStream )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )

# Component(s) in the package:
atlas_add_library( TileByteStreamLib
   TileByteStream/*.h TileByteStream/*.icc src/*.cxx
   PUBLIC_HEADERS TileByteStream
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel
   ByteStreamCnvSvcBaseLib ByteStreamData CxxUtils GaudiKernel StoreGateLib TileCalibBlobObjs
   TileConditionsLib TileEvent TileIdentifier TileL2AlgsLib
   PRIVATE_LINK_LIBRARIES CaloDetDescrLib CaloIdentifier TileDetDescr
   TileRecUtilsLib xAODEventInfo )

atlas_add_component( TileByteStream
   src/components/*.cxx
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES TileByteStreamLib )

# Install files from the package:
atlas_install_data( share/*.dump )
atlas_install_runtime( share/TileDefault.* )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( TileDigitsContByteStreamRead_test
   SCRIPT test/TileDigitsContByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileRawChannelContByteStreamRead_test
   SCRIPT test/TileRawChannelContByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileBeamElemContByteStreamRead_test
   SCRIPT test/TileBeamElemContByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileL2ContByteStreamRead_test
   SCRIPT test/TileL2ContByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileLaserObjByteStreamRead_test
   SCRIPT test/TileLaserObjByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileMuRcvContByteStreamRead_test
   SCRIPT test/TileMuRcvContByteStreamRead_test.sh
   PROPERTIES TIMEOUT 500
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TileRawChannel2Bytes4_test
                SOURCES test/TileRawChannel2Bytes4_test.cxx
                LINK_LIBRARIES TileByteStreamLib )

atlas_add_test( TileROD_Decoder_test
                SOURCES test/TileROD_Decoder_test.cxx
                LINK_LIBRARIES TileByteStreamLib IdDictParser PathResolver TestTools
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )

atlas_add_test( TileHid2RESrcIDConfig_test
                SCRIPT python -m TileByteStream.TileHid2RESrcIDConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileByteStreamConfig_test
                SCRIPT python -m TileByteStream.TileByteStreamConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)
