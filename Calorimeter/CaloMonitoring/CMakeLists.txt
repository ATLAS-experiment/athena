# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist MathCore )

# Component(s) in the package:
atlas_add_component( CaloMonitoring
   CaloMonitoring/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} CaloEvent CaloIdentifier CommissionEvent
   AthenaMonitoringLib StoreGateLib Identifier xAODCaloEvent GaudiKernel
   LArIdentifier LArRecConditions LArCablingLib MissingETEvent egammaEvent
   AthContainers LumiBlockData TrigDecisionToolLib CaloDetDescrLib
   CaloGeoHelpers AthenaKernel xAODEventInfo LArRecEvent JetEvent
   RecBackgroundEvent CaloConditions )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( TileCaloCellMonAlg_test
                SCRIPT python -m CaloMonitoring.TileCalCellMonAlg
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( LArCellMonBinning_test
                SCRIPT python -m CaloMonitoring.LArCellBinning_test 
                POST_EXEC_SCRIPT nopost.sh)
