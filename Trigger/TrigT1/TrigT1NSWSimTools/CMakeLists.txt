# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1NSWSimTools )

# External dependencies:
find_package( Boost )
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore )

# Library in the package
atlas_add_library( TrigT1NSWSimToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT1NSWSimTools
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel FourMomUtils GaudiKernel GeoPrimitives Identifier MuonCondData MuonDigitContainer MuonIdHelpersLib MuonRDO MuonReadoutGeometry MuonSimData MuonSimEvent MuonTesterTreeLib PathResolver TrkSurfaces TrackRecordLib
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AGDDKernel AtlasHepMCLib TruthUtils GeneratorObjects StoreGateLib )

# Component(s) in the package:
atlas_add_component( TrigT1NSWSimTools
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT1NSWSimToolsLib )
