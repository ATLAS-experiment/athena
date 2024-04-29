# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArG4FCAL )

# External dependencies:
find_package( Geant4 )

# Component(s) in the package:
atlas_add_library( LArG4FCAL
                   src/*.cc
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} AthenaKernel CaloG4SimLib GaudiKernel GeoModelInterfaces LArG4Code LArHV LArReadoutGeometry PathResolver RDBAccessSvcLib StoreGateLib )
set_target_properties( LArG4FCAL PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
