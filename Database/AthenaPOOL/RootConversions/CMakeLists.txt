# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootConversions )

# External dependencies:
find_package( ROOT COMPONENTS RIO Tree Core )

# Component(s) in the package:
atlas_add_library( RootConversions
                   src/*.cxx
                   PUBLIC_HEADERS RootConversions
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils
                   PRIVATE_LINK_LIBRARIES RootUtils )

atlas_add_dictionary( RootConversionsDict
                      RootConversions/RootConversionsDict.h
                      RootConversions/selection.xml
                      LINK_LIBRARIES RootConversions )
