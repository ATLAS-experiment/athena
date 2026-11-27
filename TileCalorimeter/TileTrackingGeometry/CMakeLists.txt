# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileTrackingGeometry )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_component( TileTrackingGeometry
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaBaseComps GaudiKernel TrkDetDescrInterfaces CaloDetDescrLib 
                                    StoreGateLib TileDetDescr TrkDetDescrGeoModelCnv TrkDetDescrUtils TrkGeometry TrkSurfaces 
                                    TrkVolumes CaloTrackingGeometryLib CxxUtils )
