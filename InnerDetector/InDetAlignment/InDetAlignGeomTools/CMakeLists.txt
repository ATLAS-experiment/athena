# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetAlignGeomTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Geom )

# Component(s) in the package:
atlas_add_component( InDetAlignGeomTools
   src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthContainers GeoModelInterfaces GeoModelUtilities
   InDetIdentifier InDetReadoutGeometry PixelReadoutGeometryLib RDBAccessSvcLib SCT_ReadoutGeometry
   TRT_ReadoutGeometry TrkAlignEvent TrkAlignGenToolsLib TrkAlignInterfaces TrkRIO_OnTrack )
