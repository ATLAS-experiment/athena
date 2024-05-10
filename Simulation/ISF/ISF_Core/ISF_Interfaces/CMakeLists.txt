# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_Interfaces )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( ISF_InterfacesLib
                   src/*.cxx
                   PUBLIC_HEADERS ISF_Interfaces
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel AtlasDetDescr AtlasHepMCLib CxxUtils GeoPrimitives GaudiKernel GeneratorObjects ISF_Event StoreGateLib TrackRecordLib TruthUtils )
