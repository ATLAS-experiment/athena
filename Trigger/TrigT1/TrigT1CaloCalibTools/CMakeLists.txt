# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloCalibTools )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( TrigT1CaloCalibToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT1CaloCalibTools
                   INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CORAL_LIBRARIES} AsgTools AthenaBaseComps AthenaKernel AthenaPoolUtilities CaloDetDescrLib CaloEvent CaloIdentifier CaloTriggerToolLib DerivationFrameworkInterfaces GaudiKernel Identifier LArCablingLib LArElecCalib LArIdentifier LArRawEvent LArRecConditions StoreGateLib TileConditionsLib TileEvent TrigT1CaloCalibConditions TrigT1CaloCalibToolInterfaces TrigT1CaloCondSvcLib TrigT1CaloEventLib TrigT1CaloUtilsLib TrigT1Interfaces xAODBase xAODTrigL1Calo
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} )

atlas_add_component( TrigT1CaloCalibTools
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT1CaloCalibToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
