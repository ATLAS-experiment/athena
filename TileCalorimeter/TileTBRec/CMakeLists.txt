# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileTBRec )

# External dependencies:
find_package( Boost COMPONENTS thread )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( tdaq-common )

# Component(s) in the package:
atlas_add_component( TileTBRec
                     src/Tile*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthContainers GaudiKernel TileConditionsLib TileIdentifier TileByteStreamLib CaloEvent CaloIdentifier AthenaKernel StoreGateLib ByteStreamCnvSvcBaseLib xAODEventInfo TileCalibBlobObjs TileEvent TileSimEvent TileRecUtilsLib PathResolver CxxUtils )

# Install files from the package:
atlas_install_runtime( share/TileEtaCTB.txt )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( TileTBDumpConfig_test
                SCRIPT python -m TileTBRec.TileTBDumpConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TileTBStatConfig_test
                SCRIPT python -m TileTBRec.TileTBStatConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TileTestBeamRecoConfig_test
                SCRIPT python -m TileTBRec.TileTestBeamRecoConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TileTBAANtupleConfig_test
                SCRIPT python -m TileTBRec.TileTBAANtupleConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)
