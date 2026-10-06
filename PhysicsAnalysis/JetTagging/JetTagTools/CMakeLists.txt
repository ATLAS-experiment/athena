# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( JetTagTools )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( JetTagToolsLib
    JetTagTools/*.h
    src/VertexSignificance.cxx
    src/JetFitterVariablesFactory.cxx

    PUBLIC_HEADERS JetTagTools
    PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
    PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
    LINK_LIBRARIES AthenaBaseComps GaudiKernel GeoPrimitives xAODBTagging
    xAODJet xAODTracking
    PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES}
    TrkParameters TrkTrackLink VxJetVertex VxSecVertex VxVertex )

atlas_add_component( JetTagTools
    src/components/*.cxx
    LINK_LIBRARIES JetTagToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
