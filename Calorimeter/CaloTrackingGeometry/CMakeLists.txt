# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloTrackingGeometry )

atlas_add_library( CaloTrackingGeometryLib
                   INTERFACE
                   PUBLIC_HEADERS CaloTrackingGeometry
                   LINK_LIBRARIES AthenaBaseComps CaloDetDescrLib CaloDetDescrUtils CaloGeoHelpers CaloIdentifier CxxUtils GaudiKernel GeoPrimitives SubDetectorEnvelopesLib TrkDetDescrInterfaces TrkDetDescrUtils GeoModelUtilities)

# Component(s) in the package:
atlas_add_component( CaloTrackingGeometry
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES CaloTrackingGeometryLib TileDetDescr TrkGeometry TrkGeometrySurfaces TrkSurfaces TrkVolumes)

atlas_add_dictionary( CaloTrackingGeometryDict
                      CaloTrackingGeometry/CaloTrackingGeometryDict.h
                      CaloTrackingGeometry/selection.xml
                      LINK_LIBRARIES CaloTrackingGeometryLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

