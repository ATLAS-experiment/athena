# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventSelectorAthenaPool )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
atlas_add_component( EventSelectorAthenaPool src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolCnvSvcLib AthenaPoolUtilities CxxUtils GaudiKernel PersistentDataModel PoolSvcLib StoreGateLib
		     PRIVATE_LINK_LIBRARIES CollectionSvc PersistencySvc StorageSvc )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( CondProxyProviderConfig_test
                SCRIPT python -m EventSelectorAthenaPool.CondProxyProviderConfig
                LOG_SELECT_PATTERN "ComponentAccumulator|^---|^CondProxyProvider" )

