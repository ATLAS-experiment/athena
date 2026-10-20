# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigMuon )

# External dependencies.
find_package( ROOT COMPONENTS Core GenVector )
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigMuon
    xAODTrigMuon/*.h xAODTrigMuon/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODTrigMuon
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthLinks xAODBase xAODCore xAODTracking
    PRIVATE_LINK_LIBRARIES TruthUtils )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigMuon/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::L2StandAloneMuonContainer_v1"
              "xAOD::L2StandAloneMuonContainer_v2" "xAOD::L2IsoMuonContainer_v1"
              "xAOD::L2CombinedMuonContainer_v1" )

atlas_add_dictionary( xAODTrigMuonDict
    xAODTrigMuon/xAODTrigMuonDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODTrigMuon
    EXTRA_FILES Root/dict/*.cxx )

if( NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODTrigMuonSerCnv
      FILES xAODTrigMuon/L2StandAloneMuonContainer.h xAODTrigMuon/L2StandAloneMuonAuxContainer.h
            xAODTrigMuon/L2CombinedMuonContainer.h xAODTrigMuon/L2CombinedMuonAuxContainer.h
            xAODTrigMuon/L2IsoMuonContainer.h xAODTrigMuon/L2IsoMuonAuxContainer.h
      TYPES_WITH_NAMESPACE
            xAOD::L2StandAloneMuonContainer xAOD::L2StandAloneMuonAuxContainer
            xAOD::L2CombinedMuonContainer xAOD::L2CombinedMuonAuxContainer
            xAOD::L2IsoMuonContainer xAOD::L2IsoMuonAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODTrigMuon TrigSerializeCnvSvcLib )
endif()
