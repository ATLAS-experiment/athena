# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExHiveAthenaPool )

# Component(s) in the package:
atlas_add_dictionary( AthExHiveAthenaPoolDict
                      AthExHiveAthenaPool/AthExHiveAthenaPoolDict.h
                      AthExHiveAthenaPool/selection.xml
                      LINK_LIBRARIES AthExHiveLib )

atlas_add_poolcnv_library( AthExHiveAthenaPoolCnv
                      FILES AthExHive/HiveDataObj.h
                      LINK_LIBRARIES AthExHiveLib AthenaPoolCnvSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
if( NOT XAOD_ANALYSIS )
   atlas_add_test( WriteHiveDataObj_test
                   SCRIPT python -m AthExHiveAthenaPool.WriteHiveDataObjConfig
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( ReadHiveDataObjs_test
                   SCRIPT python -m AthExHiveAthenaPool.ReadHiveDataObjs
                   DEPENDS WriteHiveDataObj_test )

   atlas_add_test( WriteHiveWithMetaData_test
                   SCRIPT python -m AthExHiveAthenaPool.WriteHiveWithMetaData
                   POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( ReadHiveWithMetaData_test
                   SCRIPT python -m AthExHiveAthenaPool.ReadHiveWithMetaData
                   DEPENDS WriteHiveWithMetaData_test POST_EXEC_SCRIPT noerror.sh )

endif()
