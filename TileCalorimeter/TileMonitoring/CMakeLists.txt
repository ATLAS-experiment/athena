# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Gpad Graf Hist HistPainter Spectrum Core Tree MathCore RIO )

# Component(s) in the package:
atlas_add_library( TileMonitoringLib
                   TileMonitoring/*.h
                   INTERFACE
                   PUBLIC_HEADERS TileMonitoring
                   LINK_LIBRARIES GaudiKernel )

atlas_add_component( TileMonitoring
  TileMonitoring/*.h src/*.cxx src/components/*.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaKernel AthenaMonitoringKernelLib AthenaMonitoringLib ByteStreamCnvSvcBaseLib CaloEvent CaloIdentifier CxxUtils GaudiKernel JetInterface JetSelectorToolsLib StoreGateLib TileCalibBlobObjs TileConditionsLib TileEvent TileIdentifier TileMonitoringLib TileRecUtilsLib TrigConfData TrigDecisionToolLib TrigT1Result xAODCaloEvent xAODCore xAODEventInfo xAODJet xAODMuon xAODTrigger Identifier )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( data/*.yaml )

# Tests:
atlas_add_test( TileCellMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileCellMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTowerMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTowerMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileClusterMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileClusterMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileMuIdMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileMuIdMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileJetMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileJetMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDQFragMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileDQFragMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileMBTSMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileMBTSMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDigiNoiseMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileDigiNoiseMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelTimeMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileRawChannelTimeMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRawChannelNoiseMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileRawChannelNoiseMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileMuonFitMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileMuonFitMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileRODMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileRODMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTMDBMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTMDBMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTMDBDigitsMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTMDBDigitsMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTMDBRawChannelMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTMDBRawChannelMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileMonitoringConfig_test
                SCRIPT python -m TileMonitoring.TileMonitoringConfig
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileMonitoring_test
                SCRIPT python -m TileMonitoring.RunTileMonitoring --evtMax=3
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDigitsMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileDigitsMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileDigitsFlxMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileDigitsFlxMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTBPulseMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTBPulseMonitorAlgorithm --nsamples 7 --demo-cabling 0 --channels '' --use-sqlite ''
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTBMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTBMonitorAlgorithm --use-sqlite ''
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileJiveXMLConfig_test
                SCRIPT python -m TileMonitoring.TileJiveXMLConfig
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileTBCellMonitorAlgorithm_test
                SCRIPT python -m TileMonitoring.TileTBCellMonitorAlgorithm
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileTBMonitoring_test
                SCRIPT python -m TileMonitoring.RunTileTBMonitoring --use-sqlite '' --evtMax 3
                PRIVATE_WORKING_DIRECTORY
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh)
