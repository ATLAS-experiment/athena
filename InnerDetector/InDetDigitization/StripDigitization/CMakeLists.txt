# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( StripDigitization )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( Boost COMPONENTS unit_test_framework )


atlas_add_library( StripDigitizationLib
                   src/*.cxx
                   #no public headers, for internal use
                   NO_PUBLIC_HEADERS 
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel PileUpToolsLib 
                    Identifier GaudiKernel SiDigitization InDetCondTools InDetRawData InDetSimEvent HitManagement 
                    GeneratorObjects SiPropertiesToolLib InDetIdentifier InDetReadoutGeometry SCT_ReadoutGeometry 
                    InDetSimData InDetConditionsSummaryService PathResolver SCT_ConditionsToolsLib 
                    MagFieldConditions StoreGateLib  )

# Component(s) in the package:
atlas_add_component( StripDigitization
                     src/components/*.cxx
                     LINK_LIBRARIES StripDigitizationLib
                    )

atlas_add_test( ITkStripFrontEnd_test
                SOURCES  test/ITkStripFrontEnd_test.cxx 
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                LINK_LIBRARIES ${Boost_LIBRARIES} StripDigitizationLib IdDictParser TestTools
                POST_EXEC_SCRIPT "nopost.sh"
              )
atlas_add_test( ITkStripAmp_test
                SOURCES src/*.cxx src/components/*.cxx test/ITkStripAmp_test.cxx 
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                LINK_LIBRARIES ${Boost_LIBRARIES} StripDigitizationLib IdDictParser TestTools
                POST_EXEC_SCRIPT "nopost.sh"
              )

               
# Install files from the package:
atlas_install_joboptions( share/*.txt )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )