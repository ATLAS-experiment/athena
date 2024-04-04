# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package
atlas_subdir( LArClusterCellDumper )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree )

# Component(s) in the package:
atlas_add_component (LArClusterCellDumper 
                     LArClusterCellDumper/*.h 
                     src/*.cxx 
                     src/components/*.cxx
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthContainers AthenaBaseComps AthenaPoolUtilities CaloCondBlobObjs CaloConditions CaloEvent CaloGeoHelpers CaloIdentifier CxxUtils GaudiKernel Identifier LArCOOLConditions LArCablingLib LArElecCalib LArIdentifier LArRawConditions LArRawEvent LArRecConditions LArSimEvent LumiBlockData StoreGateLib xAODCaloEvent xAODEgamma xAODEventInfo xAODTracking xAODTruth )

# Install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
