# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MdtCalibSvc )

# External package dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )
find_package( ROOT COMPONENTS MathCore )

# Component(s) in the package:
atlas_add_component( MdtCalibSvc
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel GeoPrimitives MagFieldConditions MagFieldElements MdtCalibData MdtCalibInterfacesLib MuonCablingData MuonCalibEvent MuonIdHelpersLib MuonReadoutGeometry PathResolver StoreGateLib )

# Install files from the package:
atlas_install_runtime( share/DC2_t0.dat share/DC2_rt.dat )


