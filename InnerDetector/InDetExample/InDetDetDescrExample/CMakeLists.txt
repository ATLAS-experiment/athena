# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetDetDescrExample )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( InDetDetDescrExample
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel StoreGateLib GeoModelInterfaces GeoPrimitives Identifier GaudiKernel InDetCondTools InDetConditionsSummaryService InDetIdentifier ReadoutGeometryBase InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry TRT_ReadoutGeometry)

# Install files from the package:
atlas_install_scripts( tools/RunPrintSiDetElements.py
                       tools/geometry_dat_to_json.py
                       tools/module_selector_from_json.py
                       POST_BUILD_CMD ${ATLAS_FLAKE8} )
