# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthXRTInterfaces )

# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, not building package AthXRTInterfaces" )
   return()
endif()

# Xilinx XRT library
# FIXME this a hack as this library is currently not distributed in CVMFS
add_library(XRTLib SHARED IMPORTED)
set_target_properties(XRTLib PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxrt_coreutil.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

# Find OpenCL
find_package(OpenCL)

# Component(s) in the package.
atlas_add_library( AthXRTInterfacesLib
   AthXRTInterfaces/*.h src/*.cxx
   INTERFACE
   INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS}
   PUBLIC_HEADERS AthXRTInterfaces
   LINK_LIBRARIES GaudiKernel XRTLib ${OpenCL_LIBRARIES})

set_target_properties( AthXRTInterfacesLib PROPERTIES
   POSITION_INDEPENDENT_CODE TRUE )