# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkDetElementBase )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_library( TrkDetElementBase
                   src/*.cxx
                   PUBLIC_HEADERS TrkDetElementBase
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoPrimitives Identifier EventPrimitives )
