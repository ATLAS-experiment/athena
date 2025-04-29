# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonRecToolInterfacesR4 )

# External dependencies:
set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package( Acts COMPONENTS Core )
    set( extra_libs ActsCore TrkGeometry)
endif()

# Component(s) in the package:
atlas_add_library( MuonRecToolInterfacesR4
                   INTERFACE
                   PUBLIC_HEADERS MuonRecToolInterfacesR4
                   LINK_LIBRARIES ${extra_libs} AthenaBaseComps AthenaKernel GaudiKernel GeoModelUtilities 
                                  GeoPrimitives Identifier TrkEventPrimitives MuonSpacePoint MuonPatternEvent)


