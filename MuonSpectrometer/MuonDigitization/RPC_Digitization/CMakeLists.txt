# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RPC_Digitization )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( RPC_Digitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AtlasHepMCLib AthenaBaseComps PileUpToolsLib xAODEventInfo GaudiKernel MuonSimData MuonSimEvent HitManagement AthenaKernel GeneratorObjects MuonCondData MuonReadoutGeometry MuonDigitContainer MuonIdHelpersLib PathResolver GeoModelInterfaces GeometryDBSvcLib EventInfoMgtLib RDBAccessSvcLib )

# Install files from the package:
atlas_install_runtime( share/G4RPC_Digitizer.txt )

