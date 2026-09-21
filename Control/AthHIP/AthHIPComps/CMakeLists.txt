# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthHIPComps)

# Check if ROCm/HIP compilation is available.
if(NOT CMAKE_HIP_COMPILER)
   message(STATUS "ROCm/HIP not available, not building package")
   return()
endif()

# Package dependencies.
find_package(hip)
find_package(vecmem COMPONENTS HIP)

# Component(s) in the package.
atlas_add_component(AthHIPComps
   src/*.hxx src/*.cxx src/components/*.cxx
   LINK_LIBRARIES GaudiKernel AthenaKernel AthHIPInterfacesLib
   AthDeviceInterfacesLib AthenaBaseComps CxxUtils
   hip::host vecmem::hip)

# Install files from the package.
atlas_install_python_modules(python/*.py)
