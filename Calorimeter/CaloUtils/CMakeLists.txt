#Copyright (C) 2020, 2021, 2022, 2024, 2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloUtils )

# External dependencies:
find_package( Boost )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( CaloUtilsLib CaloUtils/*.h CaloUtils/*.icc src/*.h src/*.cxx
   PUBLIC_HEADERS CaloUtils
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES AthenaBaseComps AthenaKernel CaloConditions CaloDetDescrLib CaloEvent CaloGeoHelpers CaloIdentifier CaloInterfaceLib CxxUtils FourMom GaudiKernel Navigation StoreGateLib xAODCaloEvent AthLinks
   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} Identifier SGTools )

atlas_add_dictionary( CaloUtilsDict
   CaloUtils/CaloUtilsDict.h CaloUtils/selection.xml
   LINK_LIBRARIES CaloUtilsLib )

# Helper variable:
set( _jobOPath "${CMAKE_CURRENT_SOURCE_DIR}/share" )
set( _jobOPath "${_jobOPath}:${CMAKE_JOBOPT_OUTPUT_DIRECTORY}" )
set( _jobOPath "${_jobOPath}:$ENV{JOBOPTSEARCHPATH}" )

# Test(s) in the package:
atlas_add_test( CaloClusterProcessor_test
   SOURCES test/CaloClusterProcessor_test.cxx
   LINK_LIBRARIES CaloUtilsLib xAODCaloEvent TestTools AthenaBaseComps )


atlas_add_test( CaloLayerCalculator_test
   SOURCES test/CaloLayerCalculator_test.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES LINK_LIBRARIES ${CLHEP_LIBRARIES} CaloUtilsLib IdDictParser TestTools
   LOG_IGNORE_PATTERN "^lar decode|mask/zero|initialize_from_dict|^ channel range|^AtlasDetectorID::"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( CaloVertexedCell_test
   SOURCES test/CaloVertexedCell_test.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} CaloUtilsLib TestTools
   LOG_IGNORE_PATTERN "^lar decode|mask/zero|initialize_from_dict|^ channel range|^AtlasDetectorID::"
   ENVIRONMENT "JOBOPTSEARCHPATH=${_jobOPath}" )

atlas_add_test( exceptions_test
   SOURCES test/exceptions_test.cxx
   LINK_LIBRARIES CaloUtilsLib )


# Install files from the package:
atlas_install_joboptions( share/*.txt )
