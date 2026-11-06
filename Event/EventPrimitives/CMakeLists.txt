# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# $Id: CMakeLists.txt 744645 2016-05-03 19:12:58Z krasznaa $
################################################################################
# Package: EventPrimitives
################################################################################

# Declare the package name:
atlas_subdir( EventPrimitives )

# External dependencies:
find_package( Eigen )

# Component(s) in the package:
atlas_add_library( EventPrimitives
   EventPrimitives/*.h
   INTERFACE
   PUBLIC_HEADERS EventPrimitives
   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
   LINK_LIBRARIES ${EIGEN_LIBRARIES} )

# Define Eigen plugins via CMake so header include order does not matter.
# (These are also defined in EventPrimitives.h.) 
target_compile_definitions( EventPrimitives INTERFACE
  EIGEN_MATRIXBASE_PLUGIN=\"EventPrimitives/AmgMatrixBasePlugin.h\"
  EIGEN_TRANSFORM_PLUGIN=\"EventPrimitives/AmgTransformPlugin.h\" )

atlas_add_test( test_plugins
   SOURCES test/test_plugins.cxx
   LINK_LIBRARIES EventPrimitives )

atlas_add_test( test_eta
   SOURCES test/test_eta.cxx
   LINK_LIBRARIES EventPrimitives )

atlas_add_test( test_AmgDefs
   SOURCES test/test_AmgDefs.cxx
   LINK_LIBRARIES EventPrimitives )

atlas_add_test( test_CovarianceHelpers
   SOURCES test/test_CovarianceHelpers.cxx
   LINK_LIBRARIES EventPrimitives )
   
atlas_add_test( test_EventPrimitivesToStringConverter
   SOURCES test/test_EventPrimitivesToStringConverter.cxx
   LINK_LIBRARIES EventPrimitives 
   POST_EXEC_SCRIPT noerror.sh
   )


