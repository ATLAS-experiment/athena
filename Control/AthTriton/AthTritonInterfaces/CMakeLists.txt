# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthTritonInterfaces )

# Component(s) in the package:
atlas_add_library( AthTritonInterfaces
    AthTritonInterfaces/*.h
    INTERFACE
    PUBLIC_HEADERS AthTritonInterfaces
    LINK_LIBRARIES GaudiKernel
)

atlas_add_dictionary( AthTritonInterfacesDict
    AthTritonInterfaces/AthTritonInterfacesDict.h
    AthTritonInterfaces/selection.xml
    LINK_LIBRARIES AthTritonInterfaces )
