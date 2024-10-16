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
foreach(det RPC TGC MDT sTGC MM)  # CSC disabled since Run-3
   atlas_add_test( RegSelTool_${det}
                   SCRIPT python -m RegionSelector.RegSelToolConfig ${det}
                   PRIVATE_WORKING_DIRECTORY
		   POST_EXEC_SCRIPT "/usr/bin/diff -u0 RegSelCondAlg_${det}.map ${CMAKE_CURRENT_SOURCE_DIR}/share/RegSelCondAlg_${det}.map.ref" )
endforeach()


foreach(det Pixel SCT TRT LAr Tile)
   atlas_add_test( RegSelTool_${det}
                   SCRIPT python -m RegionSelector.RegSelToolConfig ${det}
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )
endforeach()



