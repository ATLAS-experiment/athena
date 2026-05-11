# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py test/MdtRdoToPrepDataTool_test.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( share/*.ref )

if( NOT SIMULATIONBASE )
   # Mdt tests
   atlas_add_test( MdtRdoToPrepDataTool_test
     SCRIPT python -m MuonConfig.MdtRdoToPrepDataTool_test
     PROPERTIES TIMEOUT 600
     LOG_SELECT_PATTERN "dead tube" )
  
   # Cache alignment will change depending on whether FRONTIER_SERVER is defined.
   atlas_add_test( MuonDataDecodeTest
                   PROPERTIES TIMEOUT 1000
                   PRIVATE_WORKING_DIRECTORY
                   LOG_SELECT_PATTERN "^MdtRawDataProvi.*|^RpcRawDataProvi.*|^TgcRawDataProvi.*|^CscRawDataProvi.*|^MdtRdoTo.*|^RpcRdoTo.*|^TgcRdoTo.*|^CscRdoTo.*|^MdtROD_Decod.*|^RpcROD_Decod.*|^TgcROD_Decod.*|^CscROD_Decod.*|^MuonCache.*"
                   SCRIPT test/testMuonDataDecode.sh )

   # Adding an identical test for the ByteStream identifiable caches (and future RDO caches)
   atlas_add_test( MuonDataDecodeTest_Cache
                   PROPERTIES TIMEOUT 1000
                   PRIVATE_WORKING_DIRECTORY
                   LOG_SELECT_PATTERN "^MdtRawDataProvi.*|^RpcRawDataProvi.*|^TgcRawDataProvi.*|^CscRawDataProvi.*|^MdtRdoTo.*|^RpcRdoTo.*|^TgcRdoTo.*|^CscRdoTo.*|^MdtROD_Decod.*|^RpcROD_Decod.*|^TgcROD_Decod.*|^CscROD_Decod.*|^MuonCache.*"
                   SCRIPT test/testMuonDataDecode_Cache.sh )

   atlas_add_test( MuonBytestreamDecodeConfigTest
                   SCRIPT python -m MuonConfig.MuonBytestreamDecodeConfig
                   PRIVATE_WORKING_DIRECTORY
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( MuonCablingConfigTest
                   SCRIPT python -m MuonConfig.MuonCablingConfig
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( MuonReconstructionConfigTest
                   SCRIPT python -m MuonConfig.MuonReconstructionConfig --threads=1
                   PROPERTIES TIMEOUT 1200
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( MuonSegmentFindingConfigTest
                   SCRIPT python -m MuonConfig.MuonSegmentFindingConfig --threads=1
                   PROPERTIES TIMEOUT 900
                   POST_EXEC_SCRIPT noerror.sh )
  
   atlas_add_test( NewxAODConvertorTest
                   SCRIPT test/MuonxAODPRDCnv_test.py 
                   PROPERTIES TIMEOUT 1200
                   POST_EXEC_SCRIPT noerror.sh )
endif()
