# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootCoreUtils )

# External dependencies:
find_package( ROOT COMPONENTS Hist Tree RIO Core )

# Component(s) in the package:
atlas_add_library( RootCoreUtils
   RootCoreUtils/*.h Root/*.cxx
   PUBLIC_HEADERS RootCoreUtils
   LINK_LIBRARIES AsgMessagingLib
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_test( ut_expression
   SOURCES test/ut_expression.cxx
   LINK_LIBRARIES RootCoreUtils
   POST_EXEC_SCRIPT noerror.sh )
