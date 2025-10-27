# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SGTools )

# External dependencies:
find_package( Boost COMPONENTS timer)

# Component(s) in the package:
atlas_add_library( SGTools
                   src/*.cxx
                   PUBLIC_HEADERS SGTools
                   LINK_LIBRARIES AthenaKernel CxxUtils SGCoreLib GaudiKernel )

atlas_add_dictionary( SGToolsDict
                      SGTools/SGToolsDict.h
                      SGTools/selection.xml
                      LINK_LIBRARIES SGTools )

# Tests in the package:
atlas_add_test( VersionedKey_test
                SOURCES
                test/VersionedKey_test.cxx
                LINK_LIBRARIES SGTools )

atlas_add_test( exceptions_test
                SOURCES
                test/exceptions_test.cxx
                LINK_LIBRARIES SGTools )

atlas_add_test( StringPool_test
                SOURCES
                test/StringPool_test.cxx
                LINK_LIBRARIES SGTools TestTools )

atlas_add_test( DataProxy_test
                SOURCES
                test/DataProxy_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES SGTools SGCoreLib ${Boost_LIBRARIES} )

atlas_add_test( DataStore_test
                SOURCES
                test/DataStore_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES SGTools TestTools ${Boost_LIBRARIES} )

atlas_add_test( TransientAddress_test
                SOURCES
                test/TransientAddress_test.cxx
                LINK_LIBRARIES SGTools )

atlas_add_test( CurrentEventStore_test
                SOURCES
                test/CurrentEventStore_test.cxx
                LINK_LIBRARIES SGTools )

atlas_add_test( SGFolderItem_test
                SOURCES
                test/SGFolderItem_test.cxx
                LINK_LIBRARIES SGTools )

atlas_add_test( transientKey_test
                SOURCES
                test/transientKey_test.cxx
                LINK_LIBRARIES SGTools )
