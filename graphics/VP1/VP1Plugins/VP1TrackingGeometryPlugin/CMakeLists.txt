# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1TrackingGeometryPlugin )

# External dependencies:
find_package( Qt5 COMPONENTS Core )

# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )

# get the package name into the variable 'pkgName', to be used below
atlas_get_package_name( pkgName )

# Make the package target depend on this one:
#add_dependencies( VP1PlugUtils VP1GuideLineSystems VP1UtilitySystems VP1TrackingGeometrySystems)

# Build the library.
atlas_add_library( VP1TrackingGeometryPlugin
   VP1TrackingGeometryPlugin/*.h src/*.cxx
   PUBLIC_HEADERS VP1TrackingGeometryPlugin
   LINK_LIBRARIES Qt5::Core VP1Base
   PRIVATE_LINK_LIBRARIES VP1GuideLineSystems VP1TrackingGeometrySystems )
