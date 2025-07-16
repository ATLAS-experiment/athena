# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAPipeline)

# If XRT environment variable is not set, do not build the package
if( NOT DEFINED ENV{XILINX_XRT} )
  message( STATUS "XRT not available, skipping EFTrackingFPGAPipeline" )
  return()
endif()

find_package(OpenCL)

# Xilinx XRT library
find_package(XRT)

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

atlas_add_component(EFTrackingFPGAPipeline
                    src/*.h 
                    src/*.cxx 
                    src/components/*.cxx 
                    INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS} 
                    PUBLIC_HEADERS EFTrackingFPGAPipeline 
                    LINK_LIBRARIES GaudiKernel 
                                   AthenaBaseComps 
                                   AthXRTInterfacesLib
                                   AthenaMonitoringKernelLib
                                   xAODInDetMeasurement 
                                   ${OpenCL_LIBRARIES} 
                                   InDetPrepRawData 
                                   InDetIdentifier 
                                   InDetReadoutGeometry
                                   IRegionSelector
                                   TrigSteeringEvent
                                   LibXRT 
                                   LibXOCL
                                   FPGATrackSimObjectsLib
				   EFTrackingFPGAUtilityLib TrigSteeringEvent)

atlas_install_python_modules(python/*.py)
atlas_install_scripts(test/*.sh test/FPGARunScripts/*.sh)

