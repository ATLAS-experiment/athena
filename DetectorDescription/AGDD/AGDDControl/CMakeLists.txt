# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AGDDControl )

# External dependencies:
find_package( CLHEP )
find_package( XercesC )
find_package( GeoModelCore )
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( AGDDControl
   AGDDControl/*.h src/*.cxx
   PUBLIC_HEADERS AGDDControl
   INCLUDE_DIRS ${XercesC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}

   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   DEFINITIONS ${CLHEP_DEFINITIONS}
   LINK_LIBRARIES ${XercesC_LIBRARIES} ${CLHEP_LIBRARIES} GeoModelCore::GeoModelKernel
                  AthenaBaseComps AGDDKernel AGDDModel CxxUtils GaudiKernel
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaKernel
                          GeoModelInterfaces GeoModelUtilities PathResolver
                          StoreGateLib )