# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonGeoModel
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModel )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead )

# Component(s) in the package:
atlas_add_library( MuonGeoModelLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonGeoModel
                   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps AthenaKernel GeoModelUtilities GaudiKernel 
                                   MuonReadoutGeometry MuonGMdbObjects StoreGateLib MuonIdHelpersLib 
                                  MuonAGDDDescription AGDDControl AGDDKernel CxxUtils
                   PRIVATE_LINK_LIBRARIES AthenaPoolUtilities IdDictDetDescr MuonDetDescrUtils )

atlas_add_component( MuonGeoModel
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaKernel StoreGateLib GeoModelUtilities GaudiKernel  
                                    MuonReadoutGeometry MuonGMdbObjects AthenaPoolUtilities AGDDKernel IdDictDetDescr 
                                    MuonAGDDDescription MuonDetDescrUtils MuonIdHelpersLib MuonGeoModelLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
