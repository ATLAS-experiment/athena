# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAIntegration)
# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, not building package EFTrackingFPGAIntegration" )
   return()
endif()

find_package(OpenCL REQUIRED)
atlas_add_component(EFTrackingFPGAIntegration
    src/*.h src/*.cxx src/components/*.cxx 
    INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS} 
    LINK_LIBRARIES GaudiKernel AthenaBaseComps ${OpenCL_LIBRARIES}
)

atlas_install_python_modules(python/*.py)
