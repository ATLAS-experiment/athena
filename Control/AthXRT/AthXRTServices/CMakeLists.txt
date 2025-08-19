# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthXRTServices )

# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, not building package AthXRTServices" )
   return()
endif()

# Xilinx XRT library
# FIXME this a hack as this library is currently not distributed in CVMFS
add_library(LibXRT SHARED IMPORTED)
set_target_properties(LibXRT PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxrt_core.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

add_library(LibXOCL SHARED IMPORTED)
set_target_properties(LibXOCL PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxilinxopencl.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

# Component(s) in the package.
atlas_add_component( AthXRTServices
   src/*.hxx src/*.cxx src/*.cuh src/*.cu src/components/*.cxx
   LINK_LIBRARIES
   GaudiKernel AthXRTInterfacesLib
   AthenaBaseComps CxxUtils LibXRT LibXOCL)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
