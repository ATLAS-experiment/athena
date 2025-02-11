# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileSimAlgs )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO MathMore Matrix )

# Component(s) in the package:
atlas_add_component( TileSimAlgs
                     src/Tile*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AthenaBaseComps EventContainers PileUpToolsLib GaudiKernel TileIdentifier TileSimEvent CaloDetDescrLib CaloEvent CaloIdentifier AthAllocators AthenaKernel StoreGateLib TileCalibBlobObjs TileConditionsLib TileDetDescr TileEvent TilePulseSimulator TileRecUtilsLib PathResolver )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( TileMuonReceiverConfig_test
                SCRIPT python -m TileSimAlgs.TileMuonReceiverConfig
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileMuonReceiverDecisionConfig_test
                SCRIPT python -m TileSimAlgs.TileMuonReceiverDecisionConfig
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDigitsMakerConfig_test
                SCRIPT python -m TileSimAlgs.TileDigitsMakerConfig
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileHitVecToCntConfig_test
                SCRIPT python -m TileSimAlgs.TileHitVecToCntConfig
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileHitToTTL1Config_test
                SCRIPT python -m TileSimAlgs.TileHitToTTL1Config
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDigitizationConfig_test
                SCRIPT python -m TileSimAlgs.TileDigitizationConfig
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTilePulseSim_test
                SCRIPT python -m TileSimAlgs.RunTilePulseSim
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)
