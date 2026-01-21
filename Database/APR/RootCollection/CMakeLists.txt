# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootCollection )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core pthread RIO Tree ROOTNTuple )
find_package( VDT )

# Component(s) in the package:
atlas_add_root_dictionary( RootCollection
                           RootCollectionDictSource
                           ROOT_HEADERS RootCollection/AttributeListLayout.h RootCollection/LinkDef.h
                           EXTERNAL_PACKAGES ROOT CORAL )

atlas_add_library( RootCollection
                   src/*.cpp
                   ${RootCollectionDictSource}
                   PUBLIC_HEADERS RootCollection
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                                ${VDT_INCLUDE_DIRS} #VDT needed by RNTuple 
                   PRIVATE_INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES}
		   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaKernel CollectionSvc POOLCore
                        FileCatalog GaudiKernel PersistencySvc PersistentDataModel RootUtils AthenaBaseComps
                        CxxUtils)

atlas_add_component( RootCollectionComponents
                   src/components/*.cpp
                   NO_PUBLIC_HEADERS
		   LINK_LIBRARIES CollectionSvc RootCollection )

atlas_add_test( ttree_coll_rw_test
                SOURCES test/ttree_rw_test.cxx test/rw_test.cxx
		LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionSvc PersistentDataModel RootCollection CxxUtils
                LOG_IGNORE_PATTERN "POOLCollectionID.*Value=" )

atlas_add_test( rntuple_coll_rw_test
                SOURCES test/rntuple_rw_test.cxx test/rw_test.cxx
		LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionSvc PersistentDataModel RootCollection CxxUtils
                LOG_IGNORE_PATTERN "POOLCollectionID.*Value=" )

# wait for the test collections to be created, then sort them
atlas_add_test( sort_test
                SCRIPT test/sort_test.py
                DEPENDS ttree_coll_rw_test rntuple_coll_rw_test )
