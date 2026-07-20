# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Module defining helper CMake code for the build of Geant4-using packages.
#

# Include the required module(s).
include( CMakeParseArguments )


# Function setting up device symbol resolution on Geant4-using targets
#
# It should be called on any final target (shared library or executable)
# that links to Geant4.
#
# If simulation is using CUDA, it sets the appropriate target properties
# needed to resolve device symbols. 
#
# Usage: atlas_configure_g4_targets( TARGETS <package target(s) using Geant4> )
#
function( atlas_configure_g4_targets )
   # Parse the options given to the function:
   cmake_parse_arguments( ARG "" "" "TARGETS" ${ARGN} )

   # Check if Simulation has requested CUDA support. Not all projects
   # currently define this option.
   if(ATLAS_BUILD_SIM_CUDA)
      set_target_properties( ${ARG_TARGETS}
         PROPERTIES
	    CUDA_SEPARABLE_COMPILATION ON
	    CUDA_RESOLVE_DEVICE_SYMBOLS ON )
   endif()
endfunction()


# Mark the module as found.
set( AtlasGeant4Utilities_FOUND TRUE )

