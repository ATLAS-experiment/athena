# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArDigitization )

# External dependencies:
find_package( CLHEP )
find_package( Boost COMPONENTS container)

# Component(s) in the package:
atlas_add_library( LArDigitizationLib
                   src/LArHitEMap.cxx
                   PUBLIC_HEADERS LArDigitization
                   LINK_LIBRARIES AthenaKernel CaloDetDescrLib
                   LArCablingLib LArRawEvent
                   PRIVATE_LINK_LIBRARIES AtlasHepMCLib TruthUtils AthAllocators
                   GaudiKernel GeneratorObjects Identifier LArIdentifier )


atlas_add_component( LArDigitization
                     src/*.cxx src/components/*.cxx
                     PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                     PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${Boost_LIBRARIES}
                     AthenaBaseComps AtlasHepMCLib TruthUtils CaloIdentifier EventInfoUtils
                     GeneratorObjects Identifier LArDigitizationLib
                     LArElecCalib LArIdentifier LArRawConditions
                     LArRecConditions LArSimEvent PileUpToolsLib
                     StoreGateLib xAODEventInfo )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
