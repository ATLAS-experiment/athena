# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExStoreGateExample )

# Component(s) in the package:
atlas_add_component( AthExStoreGateExample
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthContainers AthLinks AthenaBaseComps AthenaKernel GaudiKernel
                     SGTools StoreGateLib TestTools xAODEventInfo )

atlas_add_component( AthExDFlow
                     src_dflow/*.cxx
                     src_dflow/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel SGTools StoreGateLib xAODEventInfo )

atlas_add_dictionary( AthExStoreGateExampleDict
                      AthExStoreGateExample/AthExStoreGateExampleDict.h
                      AthExStoreGateExample/selection.xml
                      LINK_LIBRARIES AthenaKernel GaudiKernel StoreGateLib )

# Tests in the package:
atlas_add_test( CircularDependency
                SCRIPT test/test_CircularDependency.py
                LOG_SELECT_PATTERN testalg )

atlas_add_test( DecorHandle
                SCRIPT test/test_DecorHandle.py
                LOG_SELECT_PATTERN testalg )

atlas_add_test( ReadWrite
                SCRIPT test/test_ReadWrite.py
                LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}" )

atlas_add_test( ReadWriteReentrant
                SCRIPT test/test_ReadWriteReentrant.py
                LOG_IGNORE_PATTERN "0x[0-9a-fA-F]{4,}" )

atlas_add_test( StoreGateDFlow
                SCRIPT test/test_DFlowAlg.py
                LOG_SELECT_PATTERN "dflow_alg"
                LOG_IGNORE_PATTERN "ptr: 0x" )
