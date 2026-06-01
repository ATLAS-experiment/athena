# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Generators/Pepper_i/CMakeLists.txt
#
# Pepper interface for ATLAS AthGeneration.
#
# This package contains no C++. It locates the pepper_kokkos binary
# from the LCG release (via FindPepper.cmake in atlas/atlasexternals)
# and provides a thin Python wrapper that invokes it from inside the
# AthGeneration Gen_tf.py transform.
#
# See doc/README.md for the runtime requirements (notably: jobs
# must run on a GPU-equipped node).

# Declare the package
atlas_subdir( Pepper_i )

# Locate pepper_kokkos. The FindPepper.cmake module is supplied by
# atlas/atlasexternals alongside the other generator find modules
# (FindPythia8.cmake, FindSherpa.cmake, etc.). Not REQUIRED so that
# releases not shipping pepper_kokkos build cleanly without this
# package.
find_package( Pepper )

if( PEPPER_FOUND )

   # Install the Python helpers
   atlas_install_python_modules( python/*.py
      POST_BUILD_CMD ${ATLAS_FLAKE8} )

   # Install the include-able JO fragment
   atlas_install_joboptions( share/common/*.py )

   # Expose the resolved Pepper executable path through the package's
   # release data so PepperConfig.py can find it without a PATH lookup.
   # (PEPPER_DATA_PATH is exported automatically by FindPepper.cmake
   # via PEPPER_ENVIRONMENT.)
   set_property( GLOBAL APPEND PROPERTY ATLAS_RELEASE_DATA
      PEPPER_EXECUTABLE=${PEPPER_pepper_EXECUTABLE} )

else()

   # If pepper_kokkos is absent we skip building this package rather
   # than aborting the entire athena configure (per @akraszna's
   # review request). However, on releases that should ship
   # pepper_kokkos but where the find module fails for some other
   # reason (broken LCG view, missing rpm, atlasexternals/find module
   # bug), this skip should not be silent (per @averbyts's review
   # request). Emit a visible warning pointing at the most likely
   # causes.
   message( WARNING
      "Pepper_i: pepper_kokkos not found by FindPepper.cmake. "
      "This package will be skipped. If pepper_kokkos is expected "
      "to be available in this release, check the LCG view setup "
      "and atlas/atlasexternals's FindPepper.cmake." )

endif()
