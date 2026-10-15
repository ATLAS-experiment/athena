# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EventSelectorAthenaPool )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
# The package is split by directory, so the two source globs can never overlap:
#   src/lib/*.cxx     -> EventSelectorAthenaPoolLib (sources and their private headers)
#   src/*.cxx         -> the component (non-recursive; also includes lib/ headers)
#
# EventSelectorAthenaPoolLib only exists because AthenaRootComps (xAODEventSelector) opens a
# pool::Collection. Its single public header is EventSelectorAthenaPool/Collection.h.
# If AthenaRootComps stops depending on Collection, drop this library together with the
# EventSelectorAthenaPool/ header directory, and add src/lib/*.cxx to the component.
atlas_add_library( EventSelectorAthenaPoolLib
                   src/lib/*.cxx
                   PUBLIC_HEADERS EventSelectorAthenaPool
                   LINK_LIBRARIES StorageSvc
                   PRIVATE_INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaKernel CxxUtils GaudiKernel
                                          PersistentDataModel )

atlas_add_component( EventSelectorAthenaPool
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolCnvSvcLib
                                    AthenaPoolUtilities EventSelectorAthenaPoolLib CxxUtils GaudiKernel
                                    PersistentDataModel PoolSvcLib SGTools StorageSvc StoreGateLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( CondProxyProviderConfig_test
                SCRIPT python -m EventSelectorAthenaPool.CondProxyProviderConfig
                LOG_SELECT_PATTERN "ComponentAccumulator|^---|^CondProxyProvider" )
