# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TileTrackingGeometry )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_component( TileTrackingGeometry
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${GEOMODEL_LIBRARIES} AthenaBaseComps GaudiKernel TrkDetDescrInterfaces CaloDetDescrLib StoreGateLib TileDetDescr TrkDetDescrGeoModelCnv TrkDetDescrUtils TrkGeometry TrkSurfaces TrkVolumes CaloTrackingGeometryLib CxxUtils )
