# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigNavigation )

# External dependencies:
find_package( ROOT COMPONENTS Core )
find_package( TBB )

# Component(s) in the package:
atlas_add_library( TrigNavigationLib
   TrigNavigation/*.h TrigNavigation/*.icc src/*.cxx
   PUBLIC_HEADERS TrigNavigation
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES AthContainers AthLinks AsgMessagingLib
   AthenaBaseComps AthenaKernel CxxUtils SGTools GaudiKernel TrigNavStructure
   TrigSerializeResultLib TrigStorageDefinitions StoreGateLib xAODCore
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${TBB_LIBRARIES} AthContainersInterfaces AthContainersRoot RootUtils
   TrigConfHLTUtilsLib TrigSerializeCnvSvcLib )

atlas_add_component( TrigNavigation
   src/components/*.cxx
   LINK_LIBRARIES TrigNavigationLib )

atlas_add_dictionary( TrigNavigationDict
  TrigNavigation/TrigNavigationDict.h TrigNavigation/selection.xml
   EXTRA_FILES test/dict/*.cxx
   LINK_LIBRARIES TrigNavigationLib )

# Test(s) in the package:
foreach( name TriggerElement_test Holder_test Registration_test Ownership_test HLTNavigation_test )

   atlas_add_test( ${name}
      SOURCES test/${name}.cxx
      LINK_LIBRARIES TrigNavigationLib AsgMessagingLib AthContainers AthContainersRoot
      AthLinks AthenaKernel StoreGateLib GaudiKernel TestTools xAODCore
      TrigSerializeCnvSvcLib
      PROPERTIES TIMEOUT 300
      ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share"
      POST_EXEC_SCRIPT noerror.sh )

endforeach()

# Install files from the package:
atlas_install_joboptions( share/test.txt )
