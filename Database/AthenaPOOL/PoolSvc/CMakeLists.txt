# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PoolSvc )


atlas_add_library( PoolSvcLib
                   INTERFACE
                   PUBLIC_HEADERS PoolSvc
                   LINK_LIBRARIES DataModelRoot GaudiKernel )

# Component(s) in the package:
atlas_add_component( PoolSvc
                     PoolSvc/*.h src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils DataModelRoot GaudiKernel PathResolver PersistentDataModel PoolSvcLib CollectionSvc PersistencySvc StorageSvc )
