# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetEvent )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( JetEvent
                   src/*.cxx
                   PUBLIC_HEADERS JetEvent
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AthContainers AthLinks Navigation AthenaKernel EventKernel FourMom NavFourMom ParticleEvent
                   PRIVATE_LINK_LIBRARIES GaudiKernel StoreGateLib )

atlas_add_dictionary( JetEventDict
                      JetEvent/JetEventDict.h
                      JetEvent/selection.xml
                      LINK_LIBRARIES AthContainers JetEvent )
