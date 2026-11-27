# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonGeometryCnv )


find_package( GeoModelCore )
find_package( GeoModelIO )
# Component(s) in the package:
atlas_add_component( MuonGeometryCnv
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers GeoModelIO::GeoModelDBManager  
                                    GeoModelIO::GeoModelWrite GeoModelIO::GeoModelIOHelpers AthenaBaseComps GeoPrimitives MuonIdHelpersLib 
                                    MuonAlignmentDataR4 MuonReadoutGeometryR4 MuonReadoutGeometry StoreGateLib TrkSurfaces)

atlas_install_python_modules( python/*.py)
