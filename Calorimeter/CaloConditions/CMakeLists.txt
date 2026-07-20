# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloConditions )

# External dependencies:
find_package( Boost COMPONENTS thread timer )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( CaloConditions
                   src/*.cxx
                   PUBLIC_HEADERS CaloConditions
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils Identifier CaloIdentifier CaloCondBlobObjs AthenaKernel GaudiKernel
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_dictionary( CaloConditionsDict
                      CaloConditions/CaloConditionsDict.h
                      CaloConditions/selection.xml
                      LINK_LIBRARIES CaloConditions )
if( CMAKE_INTERPROCEDURAL_OPTIMIZATION AND CMAKE_COMPILER_IS_GNUCXX )
  # Suppress spurious warnings seen in the LTO build.
  set_target_properties( CaloConditionsDict PROPERTIES LINK_FLAGS " -Wno-alloc-size-larger-than "  )
endif()

atlas_add_test( ToolConstants_test
                SOURCES
                test/ToolConstants_test.cxx
                LINK_LIBRARIES CaloConditions )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
