# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometryInterfaces )

# External dependencies:
set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package( Acts COMPONENTS Core )
    set( extra_libs Acts::Core TrkGeometry ActsInteropLib)
endif()

# Component(s) in the package:
atlas_add_library( ActsGeometryInterfacesLib
                   src/DetectorAlignStore.cxx
                   ActsGeometryInterfaces/*.h
                   PUBLIC_HEADERS ActsGeometryInterfaces
                   LINK_LIBRARIES ${extra_libs} AthenaBaseComps AthenaKernel GaudiKernel GeoModelUtilities GeoPrimitives 
                                  Identifier TrkEventPrimitives)
