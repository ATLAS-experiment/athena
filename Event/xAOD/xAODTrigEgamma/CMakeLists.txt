# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigEgamma )

# External dependencies.
find_package( ROOT COMPONENTS Core GenVector )
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigEgamma
    xAODTrigEgamma/*.h xAODTrigEgamma/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODTrigEgamma
    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
    LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthLinks xAODBase
    xAODCaloEvent xAODCore xAODEgamma xAODTracking xAODTrigCalo )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigEgamma/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::TrigPhotonContainer_v1" "xAOD::TrigElectronContainer_v1" )

atlas_add_dictionary( xAODTrigEgammaDict
    xAODTrigEgamma/xAODTrigEgammaDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODTrigEgamma
    EXTRA_FILES Root/dict/*.cxx )

# Serializer converter for Trigger.
IF( NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODTrigEgammaSerCnv
      FILES xAODTrigEgamma/TrigPhotonContainer.h xAODTrigEgamma/TrigPhotonAuxContainer.h
            xAODTrigEgamma/TrigElectronContainer.h xAODTrigEgamma/TrigElectronAuxContainer.h
            xAODTrigEgamma/PhotonTrigAuxContainer.h xAODTrigEgamma/ElectronTrigAuxContainer.h
      TYPES_WITH_NAMESPACE xAOD::TrigPhotonContainer xAOD::TrigPhotonAuxContainer
                           xAOD::TrigElectronContainer xAOD::TrigElectronAuxContainer
                           xAOD::PhotonTrigAuxContainer xAOD::ElectronTrigAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODTrigEgamma TrigSerializeCnvSvcLib )
endif()
