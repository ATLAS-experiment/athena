# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrackOverlayRec )

# External dependencies:
find_package( onnxruntime )

set(ALL_ONNX_LIBS ${onnxruntime_LIBRARY} ${onnxruntime_LIBRARIES} ${ONNXRUNTIME_LIBRARIES})

# Component(s) in the package:
atlas_add_component( TrackOverlayRec
                     src/*.cxx src/*.h
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${OOT_INCLUDE_DIRSRS} ${ONNXRUNTIME_INCLUDE_DIRS}
		     LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} EventBookkeeperToolsLib TrkTruthTrackInterfaces PathResolver AthOnnxruntimeServiceLib)

# Install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
