# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArTrackingGeometry )

# External dependencies:
find_package ( GeoModelCore )

# Component(s) in the package:
atlas_add_component( LArTrackingGeometry
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GeoModelCore::GeoModelKernel AthenaBaseComps CxxUtils GaudiKernel 
                                    TrkDetDescrInterfaces CaloDetDescrLib GeoModelUtilities LArReadoutGeometry 
                                    TrkDetDescrGeoModelCnv TrkDetDescrUtils TrkGeometry TrkGeometrySurfaces 
                                    TrkSurfaces TrkVolumes CaloTrackingGeometryLib )

