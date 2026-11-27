# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AGDDKernel )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_library( AGDDKernel
   AGDDKernel/*.h src/*.cxx
   PUBLIC_HEADERS AGDDKernel
   LINK_LIBRARIES GeoPrimitives GeoModelCore::GeoModelKernel )
