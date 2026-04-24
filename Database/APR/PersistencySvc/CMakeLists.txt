# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PersistencySvc )

# Component(s) in the package:
atlas_add_library( PersistencySvc
   PersistencySvc/*.h 
   src/*.cpp 
   PUBLIC_HEADERS PersistencySvc
   PRIVATE_INCLUDE_DIRS GaudiUtils
   LINK_LIBRARIES DataModelRoot StorageSvc
   PRIVATE_LINK_LIBRARIES GaudiKernel PersistentDataModel )
   
atlas_add_executable( pool_extractFileIdentifier
   utilities/pool_extractFileIdentifier.cpp
   LINK_LIBRARIES PersistencySvc )

atlas_add_executable( pool_insertFileToCatalog
   utilities/pool_insertFileToCatalog.cpp
   LINK_LIBRARIES PersistencySvc )

atlas_add_executable( FCcmd
   utilities/FCcmd.cpp
   LINK_LIBRARIES PersistencySvc )

# Install files from the package:
atlas_install_scripts( scripts/*.py )

atlas_add_test( FCcmd
   SCRIPT test/FCcmd_test.sh )
