# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( IdDictParser )

# Component(s) in the package:
atlas_add_library( IdDictParser
                   src/IdDictParser.cxx
                   PUBLIC_HEADERS IdDictParser
                   LINK_LIBRARIES IdDict XMLCoreParser )

atlas_add_executable( tid
                      test/tid.cxx
                      LINK_LIBRARIES IdDictParser Identifier )

atlas_add_executable( test_det_id
                      test/test_indet_id.cxx
                      LINK_LIBRARIES IdDictParser Identifier )

atlas_add_dictionary( IdDictParserDict
                      IdDictParser/IdDictParserDict.h
                      IdDictParser/selection.xml
                      LINK_LIBRARIES IdDictParser )
                      
atlas_add_test( IdDictParser_test
                SOURCES test/IdDictParser_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils IdDictParser
                POST_EXEC_SCRIPT nopost.sh )
                
set_property(TEST IdDictParser_IdDictParser_test_ctest PROPERTY ENVIRONMENT_MODIFICATION "XMLDEBUG=set:1")

# Install files from the package:
atlas_install_xmls( data/*.dtd data/*.xml )

