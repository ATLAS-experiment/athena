# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir( EFTrackingAlgs )

# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, not building package AthXRTServices" )
   return()
endif()

# Xilinx XRT library
# FIXME this a hack as this library is currently not distributed in CVMFS
add_library(LibXRT SHARED IMPORTED)
set_target_properties(LibXRT PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxrt_coreutil.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

add_library(LibXOCL SHARED IMPORTED)
set_target_properties(LibXOCL PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxilinxopencl.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

atlas_add_component( EFTrackingAlgs
		                 src/*.cxx
                     src/components/*.cxx
		                 LINK_LIBRARIES AthenaBaseComps 
                                    AthenaKernel 
                                    GaudiKernel 
                                    LibXRT 
                                    LibXOCL )

