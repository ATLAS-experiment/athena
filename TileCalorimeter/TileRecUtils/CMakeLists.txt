# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileRecUtils )

# External dependencies:
find_package( Boost )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( TileRecUtilsLib
                   src/Tile*.cxx
                   PUBLIC_HEADERS TileRecUtils
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} CaloConditions CaloIdentifier AthenaBaseComps AthenaKernel Identifier GaudiKernel TileEvent TileIdentifier TileConditionsLib CaloInterfaceLib CaloUtilsLib StoreGateLib xAODEventInfo LumiBlockData
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} AthAllocators CaloDetDescrLib CaloEvent CxxUtils EventContainers GeoModelInterfaces PathResolver TileCalibBlobObjs TileDetDescr TileSimEvent )

atlas_add_component( TileRecUtils
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES TileRecUtilsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_dictionary( TileRecUtilsDict
                      TileRecUtils/TileRecUtilsDict.h
                      TileRecUtils/selection.xml
                      LINK_LIBRARIES TileRecUtilsLib )

# Tests:

atlas_add_test( TileCellBuilder_test
                SCRIPT python -m TileRecUtils.TileCellBuilderTestConfig
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileCellBuilderFromHit_test
                SCRIPT python -m TileRecUtils.TileCellBuilderFromHitTestConfig
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelBuilder_test
                SCRIPT python -m TileRecUtils.TileRawChannelBuilderTestConfig
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDQstatusAlg_test
                SCRIPT python -m TileRecUtils.TileDQstatusAlgTestConfig
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDQstatusTool_test
                SCRIPT python -m TileRecUtils.TileDQstatusToolTestConfig
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelBuilderFitConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelBuilderFitConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelBuilderOptConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelBuilderOptConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelBuilderMFConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelBuilderMFConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelBuilderWienerConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelBuilderWienerConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileCellNoiseFilterConfig_test
                 SCRIPT python -m TileRecUtils.TileCellNoiseFilterConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileCellBuilderConfig_test
                 SCRIPT python -m TileRecUtils.TileCellBuilderConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDQstatusConfig_test
                 SCRIPT python -m TileRecUtils.TileDQstatusConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelCorrectionConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelCorrectionConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelMakerConfig_test
                 SCRIPT python -m TileRecUtils.TileRawChannelMakerConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileCellMakerConfig_test
                 SCRIPT python -m TileRecUtils.TileCellMakerConfig
                 PROPERTIES TIMEOUT 300
                 PRIVATE_WORKING_DIRECTORY
                 POST_EXEC_SCRIPT noerror.sh)
