# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( OutputStreamAthenaPool )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
atlas_add_component( OutputStreamAthenaPool
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolCnvSvcLib DataModelRoot EventInfo EventInfoUtils GaudiKernel PersistentDataModel StorageSvc StoreGateLib xAODEventInfo )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )

# Test(s) in the package:
atlas_add_test( SortedEventTagWriter_TTree_test
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/testSortedEventTagWriter.py --container-type ROOTTREE
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 300 )

atlas_add_test( SortedEventTagWriter_RNTuple_test
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/testSortedEventTagWriter.py --container-type ROOTRNTUPLE
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 300 )
