# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigSteeringEvent )

# Component(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_library( TrigSteeringEvent
      TrigSteeringEvent/*.h Root/*.cxx
      PUBLIC_HEADERS TrigSteeringEvent
      LINK_LIBRARIES AthContainers AsgMessagingLib RoiDescriptor xAODCore
      TrigConfHLTData )
elseif( XAOD_ANALYSIS )
   atlas_add_library( TrigSteeringEvent
      TrigSteeringEvent/*.h Root/*.cxx src/HLTExtraData.cxx src/*Result.cxx
      src/StringSerializer.cxx src/*Info.cxx src/TrigSuperRoi.cxx
      PUBLIC_HEADERS TrigSteeringEvent
      LINK_LIBRARIES AthContainers AsgMessagingLib AthenaKernel RoiDescriptor
      xAODCore GaudiKernel TrigConfHLTData )

   atlas_add_dictionary( TrigSteeringEventDict
      TrigSteeringEvent/TrigSteeringEventDict.h TrigSteeringEvent/selection.xml
      DATA_LINKS TrigRoiDescriptor TrigSuperRoi
      ELEMENT_LINKS TrigRoiDescriptorCollection
      LINK_LIBRARIES TrigSteeringEvent )
else()
   # External dependencies:
   find_package( Boost )
   find_package( tdaq-common )

   atlas_add_library( TrigSteeringEvent
      TrigSteeringEvent/*.h Root/*.cxx src/*.cxx
      PUBLIC_HEADERS TrigSteeringEvent
      INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
      LINK_LIBRARIES ${Boost_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} AthContainers AsgMessagingLib AthenaBaseComps AthenaKernel GaudiKernel RoiDescriptor TrigConfHLTData xAODCore
      PRIVATE_LINK_LIBRARIES TrigT1Interfaces xAODTrigger TrigNavigationLib )

   atlas_add_dictionary( TrigSteeringEventDict
      TrigSteeringEvent/TrigSteeringEventDict.h TrigSteeringEvent/selection.xml
      DATA_LINKS TrigRoiDescriptor TrigSuperRoi
      ELEMENT_LINKS TrigRoiDescriptorCollection
      LINK_LIBRARIES TrigSteeringEvent )

   # Second dictionary for athena-only classes.
   atlas_add_dictionary( TrigSteeringEventAthenaDict
      TrigSteeringEvent/TrigSteeringEventAthenaDict.h TrigSteeringEvent/selection_athena.xml
      LINK_LIBRARIES TrigSteeringEvent )

   atlas_add_sercnv_library( TrigSteeringEventSerCnv
      FILES TrigSteeringEvent/TrigRoiDescriptor.h
      TrigSteeringEvent/TrigSuperRoi.h
      TrigSteeringEvent/TrigPassFlags.h
      TrigSteeringEvent/TrigPassFlagsCollection.h
      TrigSteeringEvent/TrigRoiDescriptorCollection.h
      TrigSteeringEvent/TrigSuperRoiCollection.h
      TrigSteeringEvent/TrigOperationalInfo.h
      TrigSteeringEvent/TrigOperationalInfoCollection.h
      TrigSteeringEvent/TrigPassBits.h
      TrigSteeringEvent/TrigPassBitsCollection.h
      TrigT1Interfaces/RecJetRoI.h
      TrigT1Interfaces/RecEmTauRoI.h
      TrigT1Interfaces/RecEnergyRoI.h
      TrigT1Interfaces/RecJetEtRoI.h
      TrigT1Interfaces/RecMuonRoI.h
      xAODTrigger/TrigCompositeContainer.h
      xAODTrigger/TrigCompositeAuxContainer.h
      TYPES_WITH_NAMESPACE LVL1::RecJetRoI LVL1::RecJetEtRoI LVL1::RecEmTauRoI
      LVL1::RecEnergyRoI LVL1::RecMuonRoI  xAOD::TrigCompositeContainer
      xAOD::TrigCompositeAuxContainer
      LINK_LIBRARIES TrigSteeringEvent TrigT1Interfaces xAODTrigger TrigSerializeCnvSvcLib )

   atlas_add_sercnv_library( TrigSteeringEventxAODSerCnv
      FILES xAODTrigger/TrigPassBitsContainer.h
      xAODTrigger/TrigPassBitsAuxContainer.h
      TYPES_WITH_NAMESPACE xAOD::TrigPassBits xAOD::TrigPassBitsContainer
      xAOD::TrigPassBitsAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODTrigger TrigSerializeCnvSvcLib )
endif()

atlas_generate_cliddb( TrigSteeringEvent )

# Test(s) in the package:
atlas_add_test( Enums_test
   SOURCES test/Enums_test.cxx
   LINK_LIBRARIES TrigSteeringEvent
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( TrigPassBits_test
   SOURCES test/TrigPassBits_test.cxx
   LINK_LIBRARIES TrigSteeringEvent
   POST_EXEC_SCRIPT noerror.sh )

if( NOT XAOD_STANDALONE AND NOT XAOD_ANALYSIS )
   atlas_add_test( Operators_test
      SOURCES test/Operators_test.cxx
      LINK_LIBRARIES AthenaKernel GaudiKernel TrigSteeringEvent
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Truncation_test
      SOURCES test/Truncation_test.cxx
      LINK_LIBRARIES AthenaKernel GaudiKernel TrigSteeringEvent
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( HLTExtraData_test
      SOURCES test/HLTExtraData_test.cxx
      LINK_LIBRARIES TrigSteeringEvent
      POST_EXEC_SCRIPT noerror.sh )
endif()
