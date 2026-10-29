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
# Unfortunately atlas_add_dictionary passes everything through an extra layer of shell
# string processing so the double quotes around the path would disappear. We can get around
# this by exploiting the fact that atlas_add_dictionary only uses compile_definitions, while
# the compiler gets compile_definitions *then* compile_options.

# Add outer single quotes so bash does not strip the double quotes in genreflex:
target_compile_definitions( EventPrimitives INTERFACE
   [[EIGEN_MATRIXBASE_PLUGIN='"EventPrimitives/AmgMatrixBasePlugin.h"']]
   [[EIGEN_TRANSFORM_PLUGIN='"EventPrimitives/AmgTransformPlugin.h"']] )
# In g++, undefine (-U) the single-quoted macros from above and redefine (-D) them
# with only double quotes:
target_compile_options( EventPrimitives INTERFACE
   -UEIGEN_MATRIXBASE_PLUGIN
   [[-DEIGEN_MATRIXBASE_PLUGIN="EventPrimitives/AmgMatrixBasePlugin.h"]]
   -UEIGEN_TRANSFORM_PLUGIN
   [[-DEIGEN_TRANSFORM_PLUGIN="EventPrimitives/AmgTransformPlugin.h"]] )

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


