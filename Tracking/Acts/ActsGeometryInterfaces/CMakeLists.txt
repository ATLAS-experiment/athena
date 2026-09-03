# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeometryInterfaces )

# External dependencies:
set( extra_libs )
if( NOT SIMULATIONBASE )
    find_package( Acts COMPONENTS Core )
    set( extra_libs Acts::Core TrkGeometry ActsInteropLib)
endif()

# The conversion of the Gen3 Acts::TrackingGeometry into a Detray geometry is
# only available if ACTS was built with the Detray plugin.
set( _actsGeometryDetrayDefinitions )
set( _actsGeometryDetrayLibraries )
if( TARGET Acts::PluginDetray )
  list( APPEND _actsGeometryDetrayDefinitions ACTSGEOMETRY_HAVE_DETRAY )
  list( APPEND _actsGeometryDetrayLibraries Acts::PluginDetray )
endif()

# Component(s) in the package:
atlas_add_library( ActsGeometryInterfacesLib
                   src/TransformStore.cxx
                   ActsGeometryInterfaces/*.h
                   PUBLIC_HEADERS ActsGeometryInterfaces
                   DEFINITIONS ${_actsGeometryDetrayDefinitions}
                   LINK_LIBRARIES ${extra_libs} ${_actsGeometryDetrayLibraries} AthenaBaseComps AthenaKernel GaudiKernel GeoModelUtilities GeoPrimitives 
                                  Identifier TrkEventPrimitives)


atlas_add_dictionary( ActsGeometryInterfacesDict
                      ActsGeometryInterfaces/GeometryDefs.h
                      ActsGeometryInterfaces/selection.xml 
                      LINK_LIBRARIES ActsGeometryInterfacesLib)