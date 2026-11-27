# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BCMPrimeReadoutGeometry )

# External dependencies:
find_package( GeoModelCore )

atlas_add_library( BCMPrimeReadoutGeometry
  src/*.cxx
  PUBLIC_HEADERS BCMPrimeReadoutGeometry
  LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaKernel GeoPrimitives
  PRIVATE_LINK_LIBRARIES AthenaBaseComps AthenaPoolUtilities )

