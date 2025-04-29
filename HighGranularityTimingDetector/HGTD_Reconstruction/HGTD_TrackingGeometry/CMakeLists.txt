# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_TrackingGeometry )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_component( HGTD_TrackingGeometry
   src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS}
   LINK_LIBRARIES ${GEOMODEL_LIBRARIES} GaudiKernel
   AthenaKernel AthenaBaseComps StoreGateLib GeoPrimitives Identifier
   TrkGeometry TrkSurfaces TrkVolumes TrkDetDescrInterfaces TrkDetDescrUtils
   SubDetectorEnvelopesLib HGTD_ReadoutGeometry HGTD_Identifier )
