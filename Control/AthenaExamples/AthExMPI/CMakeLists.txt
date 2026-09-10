# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthExMPI)

# External dependencies.
find_package(bmpi3 REQUIRED)
find_package(TBB REQUIRED)

# Public interface(s) in the package.
atlas_add_library(AthExMPILib
   INTERFACE
   PUBLIC_HEADERS AthExMPI)

# Component(s) in the package.
atlas_add_component(AthExMPI
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} bmpi3::bmpi3 AthExMPILib AthenaBaseComps
                  AthenaKernel CxxUtils GaudiKernel)

# Install files from the package.
atlas_install_python_modules(python/*.py)

atlas_add_test(RPCFunctionEntry_test
   SOURCES test/RPCFunctionEntry_test.cxx
   LINK_LIBRARIES AthenaKernel CxxUtils)
