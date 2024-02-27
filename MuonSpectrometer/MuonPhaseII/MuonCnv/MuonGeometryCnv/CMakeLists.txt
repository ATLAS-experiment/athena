# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonGeometryCnv )


# Component(s) in the package:
atlas_add_component( MuonGeometryCnv
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib MuonAlignmentDataR4
                                     MuonReadoutGeometryR4 MuonReadoutGeometry MuonStationGeoHelpersLib StoreGateLib TrkSurfaces)

atlas_install_python_modules( python/*.py)
