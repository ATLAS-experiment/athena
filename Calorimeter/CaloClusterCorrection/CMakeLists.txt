# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloClusterCorrection )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( CaloClusterCorrectionLib
                   src/*.cxx
                   PUBLIC_HEADERS CaloClusterCorrection
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES AthenaKernel CaloConditions CaloEvent CaloInterfaceLib CaloUtilsLib CxxUtils GaudiKernel LArRecConditions StoreGateLib TileConditionsLib xAODCaloEvent AthOnnxInterfaces
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthContainers AthenaBaseComps CaloDetDescrLib CaloGeoHelpers CaloIdentifier PathResolver)

atlas_add_component( CaloClusterCorrection
                     src/components/*.cxx
                     LINK_LIBRARIES CaloClusterCorrectionLib )


atlas_add_dictionary( CaloClusterCorrectionDict
   CaloClusterCorrection/CaloClusterCorrectionDict.h CaloClusterCorrection/selection.xml
   LINK_LIBRARIES CaloClusterCorrectionLib )

# Test(s) in the package:
atlas_add_test( interpolate_test
                SOURCES
                test/interpolate_test.cxx
                LINK_LIBRARIES CaloClusterCorrectionLib )

atlas_add_test( config_test
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/config_t.py
                LOG_IGNORE_PATTERN "Current filenames|by peeking|^Py:MetaReader|^Py:AutoConfigFlags|no dictionary for" )

atlas_add_test( CaloScaleCluster_test
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/CaloScaleCluster_test.py
                PROPERTIES TIMEOUT 300
                LOG_IGNORE_PATTERN "Reading file|no dictionary for class|by peeking|IdDictDetDescrCnv|Unable to locate catalog|Current filenames|Global tag|Cache alignment|^Py:MetaReader|AthDictLoaderSvc +INFO" )


atlas_add_test( ToolWithConstants_test
   SCRIPT test/ToolWithConstants_test.py
   LOG_IGNORE_PATTERN "no dictionary for|by peeking into|Current filenames:|IOVDbSvc +INFO|Added successfully Conversion|DetDescrCnvSvc +INFO|GeoModelSvc +INFO|IOVSvc +INFO|with default tag|^Py:MetaReader" )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
atlas_install_data( share/*.onnx )
atlas_install_data( share/*.yaml )