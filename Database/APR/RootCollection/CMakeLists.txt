# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RootCollection )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS MathCore Graf Hist Tree TreePlayer Net RIO Core pthread ROOTNTuple ROOTVecOps )
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
                   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaKernel CollectionBase
                   FileCatalog GaudiKernel POOLCore PersistencySvc PersistentDataModel RootUtils )

atlas_add_library( RootCollectionComponents
                   src/components/*.cpp
                   NO_PUBLIC_HEADERS
                   LINK_LIBRARIES CollectionBase RootCollection )

atlas_add_test( read_test
                SOURCES
                test/read_test.cxx
                LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionBase PersistentDataModel RootCollection CxxUtils
                LOG_IGNORE_PATTERN "POOLCollectionID.*Value=" )

atlas_add_test( update_test
                SOURCES
                test/update_test.cxx
                LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionBase PersistentDataModel RootCollection CxxUtils
                DEPENDS read_test )

# Component list generation:
atlas_generate_componentslist( RootCollectionComponents )
