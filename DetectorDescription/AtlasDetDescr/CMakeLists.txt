# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AtlasDetDescr )

find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_library( AtlasDetDescr
                   src/AtlasDetectorID.cxx
                   src/AtlasDetectorIDHelper.cxx
                   src/AtlasRegionHelper.cxx
                   PUBLIC_HEADERS AtlasDetDescr
                   LINK_LIBRARIES IdDict Identifier AthenaKernel AthenaBaseComps
                   PRIVATE_LINK_LIBRARIES GaudiKernel )

atlas_add_dictionary( AtlasDetDescrDict
                      AtlasDetDescr/AtlasDetDescrDict.h
                      AtlasDetDescr/selection.xml
                      LINK_LIBRARIES AtlasDetDescr )

# Tests in the package:
atlas_add_test( AtlasDetectorID_test                
                SOURCES
                test/AtlasDetectorID_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}  
                LINK_LIBRARIES  AtlasDetDescr IdDictParser ${Boost_LIBRARIES} 
                POST_EXEC_SCRIPT "nopost.sh" )
