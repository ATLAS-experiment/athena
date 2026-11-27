# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArReadoutGeometry )

# External dependencies:
find_package( CLHEP )
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_library( LArReadoutGeometry
                   src/*.cxx
                   PUBLIC_HEADERS LArReadoutGeometry
                   PRIVATE_INCLUDE_DIRS  ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaKernel GeoPrimitives Identifier LArHV StoreGateLib CxxUtils
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} GeoModelUtilities GaudiKernel RDBAccessSvcLib GeoModelInterfaces )

