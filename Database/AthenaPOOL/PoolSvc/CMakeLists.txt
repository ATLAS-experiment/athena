# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PoolSvc )


atlas_add_library( PoolSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS PoolSvc
                   LINK_LIBRARIES AthenaBaseComps CxxUtils DataModelRoot GaudiKernel PathResolver PersistentDataModel StorageSvc )

# Component(s) in the package:
atlas_add_component( PoolSvc
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils DataModelRoot GaudiKernel PathResolver PersistentDataModel PoolSvcLib StorageSvc )

atlas_add_executable( pool_extractFileIdentifier
   utilities/pool_extractFileIdentifier.cxx
   LINK_LIBRARIES CxxUtils PoolSvcLib StorageSvc )

atlas_add_executable( pool_insertFileToCatalog
   utilities/pool_insertFileToCatalog.cxx
   LINK_LIBRARIES CxxUtils PoolSvcLib StorageSvc )

atlas_add_executable( FCcmd
   utilities/FCcmd.cxx
   LINK_LIBRARIES PoolSvcLib StorageSvc )

# Install files from the package:
atlas_install_scripts( scripts/*.py )

atlas_add_test( FCcmd
   SCRIPT test/FCcmd_test.sh )
