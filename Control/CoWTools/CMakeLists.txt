# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CoWTools )

# Component(s) in the package:
atlas_add_library( CoWTools
   CoWTools/*.h src/*.cxx
   PUBLIC_HEADERS CoWTools
   LINK_LIBRARIES GaudiKernel )

if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16 )
  # Work around gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=122197
  set_target_properties( CoWTools PROPERTIES COMPILE_FLAGS " --param max-devirt-targets=1 " )
endif()
