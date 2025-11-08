# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonGeometryCnv )


find_package( GeoModel COMPONENTS GeoModelKernel GeoModelHelpers GeoModelIOHelpers GeoModelDBManager  GeoModelWrite GeoModelIOHelpers )
# Component(s) in the package:
atlas_add_component( MuonGeometryCnv
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES}  AthenaBaseComps GeoPrimitives MuonIdHelpersLib 
                                    MuonAlignmentDataR4 MuonReadoutGeometryR4 MuonReadoutGeometry StoreGateLib TrkSurfaces)

atlas_install_python_modules( python/*.py)
