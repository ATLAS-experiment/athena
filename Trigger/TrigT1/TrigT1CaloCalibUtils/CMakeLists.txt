# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloCalibUtils )

# External dependencies:
find_package( CLHEP )
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core Hist RIO )

# Component(s) in the package:
atlas_add_library( TrigT1CaloCalibUtilsLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT1CaloCalibUtils
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES AthenaBaseComps CaloDetDescrLib CaloEvent CaloIdentifier GaudiKernel LArCablingLib LArElecCalib StoreGateLib TrigT1CaloCalibConditions TrigT1CaloEventLib TrigT1CaloUtilsLib xAODTrigL1Calo
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaKernel AthenaPoolUtilities Identifier TrigT1CaloCalibToolInterfaces TrigT1CaloToolInterfaces TrigT1Interfaces xAODEventInfo )

atlas_add_component( TrigT1CaloCalibUtils
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT1CaloCalibUtilsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.txt share/*.py )

