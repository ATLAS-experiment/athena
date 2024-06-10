# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( StoreGateTests )

# Component(s) in the package:
atlas_add_library( StoreGateTestsLib
                   StoreGateTests/PayLoad.h
                   INTERFACE
                   PUBLIC_HEADERS StoreGateTests
                   LINK_LIBRARIES AthContainers AthenaKernel )

atlas_add_component( StoreGateTests
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthAllocators AthenaBaseComps GaudiKernel StoreGateLib StoreGateTestsLib )

atlas_add_dictionary( StoreGateTestsDict
                      StoreGateTests/StoreGateTestsDict.h
                      StoreGateTests/selection.xml
                      LINK_LIBRARIES StoreGateTestsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( PyClidsTestWriter
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/test_recordStlClids.py
                PROPERTIES TIMEOUT 600
                LOG_IGNORE_PATTERN "running|XMLCatalog|0x[0-9a-f]{4,}" )

atlas_add_test( SgProducerConsumer
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/test_sgProducerConsumer.py
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( PySgProducerConsumer
                SCRIPT athena.py ${CMAKE_CURRENT_SOURCE_DIR}/test/test_sgProducerConsumer.py Test.AlgMode="py_py"
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT noerror.sh )
