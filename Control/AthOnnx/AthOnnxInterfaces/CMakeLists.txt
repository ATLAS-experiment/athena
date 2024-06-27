# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthOnnxInterfaces )

# External dependencies.
find_package( onnxruntime )

# Component(s) in the package:
atlas_add_library( AthOnnxInterfaces
                   AthOnnxInterfaces/*.h
                   INTERFACE
                   PUBLIC_HEADERS AthOnnxInterfaces
                   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ONNXRUNTIME_LIBRARIES} AsgTools AsgServicesLib
                  )

atlas_add_dictionary( AthOnnxInterfacesDict
    AthOnnxInterfaces/AthOnnxInterfacesDict.h
    AthOnnxInterfaces/selection.xml
    LINK_LIBRARIES AthOnnxInterfaces )
