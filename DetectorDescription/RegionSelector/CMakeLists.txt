# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RegionSelector )

# Component(s) in the package:
atlas_add_library( RegionSelectorLib
                   src/*.cxx
                   PUBLIC_HEADERS RegionSelector
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel IRegionSelector Identifier RegSelLUT
                   PRIVATE_LINK_LIBRARIES PathResolver )

atlas_add_component( RegionSelector
                     src/components/*.cxx
                     LINK_LIBRARIES RegionSelectorLib )

atlas_add_component( RegionSelectorTest
                     test/components/*.cxx
                     test/*.cxx
                     LINK_LIBRARIES RegionSelectorLib RoiDescriptor )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Define unit tests
foreach(det Pixel SCT TRT LAr Tile RPC TGC MDT STGC MM)  # CSC disabled since Run-3
   atlas_add_test( RegSelTool_${det}
                   SCRIPT python -m RegionSelector.RegSelToolConfig ${det}
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )
endforeach()

atlas_add_test( RegSelTool_All
                SCRIPT python -m RegionSelector.RegSelToolConfig Pixel SCT TRT LAr Tile RPC TGC MDT STGC MM
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh )
