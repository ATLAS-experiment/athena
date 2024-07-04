# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcUtils )

find_package( ROOT )

# Component(s) in the package:
atlas_add_library( ZdcUtilsLib
                   Root/*.cxx
                   PUBLIC_HEADERS ZdcUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES})
