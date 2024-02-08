# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometryInterfaces )

# External dependencies:
set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package( Acts COMPONENTS Core )
    set( extra_libs ActsCore TrkGeometry)
endif()

# Component(s) in the package:
atlas_add_library( ActsGeometryInterfacesLib
                   ActsGeometryInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS ActsGeometryInterfaces
                   LINK_LIBRARIES
		      ${extra_libs}
                      AthenaBaseComps
                      AthenaKernel
		      GaudiKernel
		      GeoModelUtilities
		      GeoPrimitives
		      Identifier
		      TrkEventPrimitives )
