# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthExSYCL )

# Make use of the SYCL support in VecMem for building this package.
find_package( vecmem COMPONENTS LANGUAGE )

# Check if SYCL is available as a language.
vecmem_check_language( SYCL )
if( NOT CMAKE_SYCL_COMPILER )
   message( STATUS "SYCL is not available, not building AthExSYCL" )
   return()
endif()

# Enable SYCL as a language.
list( APPEND CMAKE_MODULE_PATH "${vecmem_CMAKE_DIR}/sycl" )
enable_language( SYCL )

# Add a component library that has some SYCL code in it.
atlas_add_component( AthExSYCL
   src/*.h src/*.cxx src/*.sycl src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps GaudiKernel )

# Install extra files from the package.
atlas_install_joboptions( share/*.py )

# Test(s) in the package.
atlas_add_test( LinearTransformExample
   SCRIPT athena.py --stdcmalloc AthExSYCL/LinearTransformExample_jobOptions.py
   POST_EXEC_SCRIPT noerror.sh )
