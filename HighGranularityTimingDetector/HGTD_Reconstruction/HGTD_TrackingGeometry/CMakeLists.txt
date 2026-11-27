# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_TrackingGeometry )

# External dependencies:
find_package( GeoModelCore )

# Component(s) in the package:
atlas_add_component( HGTD_TrackingGeometry
   src/*.cxx src/components/*.cxx
   LINK_LIBRARIES GeoModelCore::GeoModelKernel GaudiKernel
   AthenaKernel AthenaBaseComps StoreGateLib GeoPrimitives Identifier
   TrkGeometry TrkSurfaces TrkVolumes TrkDetDescrInterfaces TrkDetDescrUtils
   SubDetectorEnvelopesLib HGTD_ReadoutGeometry HGTD_Identifier )
