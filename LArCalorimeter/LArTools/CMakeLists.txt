# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArTools )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel )
find_package( ROOT COMPONENTS RIO Core )

# Component(s) in the package:

atlas_add_component( LArTools
                     src/*.cxx src/components/*.cxx 
                     LINK_LIBRARIES AthenaBaseComps Identifier LArElecCalib CaloInterfaceLib
		     ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} CaloIdentifier AthenaPoolUtilities LArRecConditions )

