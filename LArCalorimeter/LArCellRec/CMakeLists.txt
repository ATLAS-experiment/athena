# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArCellRec )

# External dependencies:
find_package( CLHEP )

#Component(s) in the package:
atlas_add_component( LArCellRec
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} CaloConditions CaloDetDescrLib CaloEvent CaloIdentifier CaloInterfaceLib
		             CaloUtilsLib AthAllocators AthenaBaseComps AthenaKernel StoreGateLib CxxUtils Identifier GaudiKernel LArIdentifier
		             LArRawEvent LArRecConditions AthenaPoolUtilities xAODEventInfo
		             LArRecEvent LArCablingLib LArElecCalib
                             PersistentDataModel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
