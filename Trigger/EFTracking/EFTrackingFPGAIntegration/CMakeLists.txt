# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAIntegration)
# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
   message( STATUS "XRT not available, building only a subset of tools and algo that are avaliable in EFTrackingFPGAIntegration" )

   # The nominal setup for this package require OPENCL, which is only currently avaliable in docker images
   # And not distributed within normal athena setup. However, this tool and alg are not dependant on that
   # and are useful to be implemented in FPGATrackSim for FPGA test vector generation

    atlas_add_component(EFTrackingFPGAIntegration
    src/FPGA*.h src/FPGA*.cxx src/components/FPGA*.cxx 
    LINK_LIBRARIES GaudiKernel AthenaBaseComps xAODInDetMeasurement InDetPrepRawData InDetIdentifier InDetReadoutGeometry
    )
    atlas_install_python_modules(python/*.py)

   return()
endif()

find_package(OpenCL REQUIRED)
atlas_add_component(EFTrackingFPGAIntegration
    src/*.h src/*.cxx src/components/*.cxx 
    INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS} 
    LINK_LIBRARIES GaudiKernel AthenaBaseComps xAODInDetMeasurement ${OpenCL_LIBRARIES} InDetPrepRawData InDetIdentifier InDetReadoutGeometry
)

atlas_install_python_modules(python/*.py)
