# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthExXRT )

# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, not building package AthExXRT" )
   return()
endif()

add_library(LibXOCL SHARED IMPORTED)
set_target_properties(LibXOCL PROPERTIES
  IMPORTED_LOCATION "$ENV{XILINX_XRT}/lib/libxilinxopencl.so"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{XILINX_XRT}/include"
)

# Add a component library that has some XRT code in it.
atlas_add_component( AthExXRT
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps GaudiKernel LibXOCL AthXRTInterfacesLib )

# Install extra files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


## Test(s) in the package.
## Currently disabled, as no FPGA bitstream present in the repo and
## automated bitstream building is not in place yet.
#atlas_add_test( XRTVectorAdditionTaskMT
#   SCRIPT athena.py --threads=4 AthExXRT/VectorAdditionExample_jobOptions.py
#   POST_EXEC_SCRIPT nopost.sh )
