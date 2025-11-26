# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(SGCore)

# Declare the component(s) of the package.
atlas_add_library(SGCoreLib
   SGCore/*.h
   INTERFACE
   PUBLIC_HEADERS SGCore)

atlas_add_dictionary(SGCoreDict
   SGCore/SGCoreDict.h
   SGCore/selection.xml
   LINK_LIBRARIES SGCoreLib)
