# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# This package is a collection of 'duel-use' tools to calculate
# high-level flavor tagging discriminants. Because it should work both
# inside and outside Athena, nothing here can use the magnetic field,
# atlas geometry, or material maps, but neural networks etc are all
# fine.

# Declare the package name:
atlas_subdir( ONNXUtils )

# External dependencies:
find_package( onnxruntime )

# Build a shared library:
atlas_add_library( ONNXUtils Root/*cxx ONNXUtils/*h
  PUBLIC_HEADERS ONNXUtils
  INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
  PRIVATE_LINK_LIBRARIES PathResolver CxxUtils
  LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} PathResolver
  )


#if (NOT XAOD_STANDALONE)
#  atlas_add_component(
#    LINK_LIBRARIES ONNXUtils
#    )
#endif()

atlas_add_dictionary( ONNXUtilsDict
   ONNXUtils/ONNXUtilsDict.h
   ONNXUtils/selection.xml
   LINK_LIBRARIES ONNXUtils 
)

#atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# add onnx metadata executable
atlas_add_executable( extract-onnx-metadata
                      utils/extract-onnx-metadata.cxx
                      INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} PathResolver ONNXUtils)
