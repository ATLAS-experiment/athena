# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT2CaloCommon )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )
find_package( TBB )

# Component(s) in the package:
atlas_add_library( TrigT2CaloCommonLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT2CaloCommon
                   PRIVATE_INCLUDE_DIRS ${TBB_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib CaloEvent CaloGeoHelpers CaloIdentifier GaudiKernel IRegionSelector Identifier LArByteStreamLib LArCablingLib LArRawUtilsLib LArRecEvent TileByteStreamLib TileEvent xAODEventInfo xAODTrigCalo LArRawConditions LArRecConditions
                   PRIVATE_LINK_LIBRARIES ${TBB_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} ByteStreamCnvSvcBaseLib CaloDetDescrLib CaloUtilsLib LArBadChannelToolLib LArIdentifier LArRecUtilsLib StoreGateLib TestTools TrigSteeringEvent )

atlas_add_component( TrigT2CaloCommon
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT2CaloCommonLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )

# Tests:
atlas_add_test( TrigCaloDataAccessSvc
                SCRIPT python -m TrigT2CaloCommon.TrigCaloDataAccessConfig
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 1200
                POST_EXEC_SCRIPT noerror.sh )
