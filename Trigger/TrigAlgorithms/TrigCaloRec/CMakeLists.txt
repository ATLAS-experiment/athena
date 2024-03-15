# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigCaloRec )

# Component(s) in the package:
atlas_add_component( TrigCaloRec
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_INCLUDE_DIRS TrigT2CaloEgamma
                     LINK_LIBRARIES AthContainers AthenaBaseComps AthenaMonitoringKernelLib CaloEvent CaloInterfaceLib CaloUtilsLib EventKernel GaudiKernel NavFourMom StoreGateLib TileConditionsLib TrigSteeringEvent TrigT2CaloCommonLib TrigT2CaloEgammaLib xAODCaloEvent xAODCore xAODTrigCalo xAODHIEvent xAODTrigRinger TrigT2CaloEgammaLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )

atlas_add_test( TrigCaloRecConfig
                 SCRIPT python -m TrigCaloRec.TrigCaloRecConfig
                 POST_EXEC_SCRIPT noerror.sh)

