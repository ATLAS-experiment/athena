# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODEgamma )

# External dependencies.
find_package( ROOT COMPONENTS Core GenVector )
find_package( xAODUtilities )

# Component(s) in the package:
atlas_add_library( xAODEgamma
   xAODEgamma/*.h xAODEgamma/versions/*.h Root/*.cxx
   PUBLIC_HEADERS xAODEgamma
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} TruthUtils AthContainers AthLinks GeoPrimitives
   EventPrimitives xAODBase xAODCaloEvent xAODCore xAODPrimitives xAODTracking xAODPFlow
   xAODTruth )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODEgamma/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::EgammaContainer_v1" "xAOD::ElectronContainer_v1"
              "xAOD::PhotonContainer_v1" )

atlas_add_dictionary( xAODEgammaDict
   xAODEgamma/xAODEgammaDict.h
   ${_selectionFile}
   LINK_LIBRARIES xAODCore xAODEgamma
   EXTRA_FILES Root/dict/*.cxx )

atlas_add_dictionary( xAODEgammaParametersDict
   xAODEgamma/xAODEgammaParametersDict.h
   xAODEgamma/selectionParameters.xml )

# Serializer converter for Trigger:
IF( NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODEgammaSerCnv
      FILES xAODEgamma/PhotonContainer.h xAODEgamma/PhotonAuxContainer.h
            xAODEgamma/ElectronContainer.h xAODEgamma/ElectronAuxContainer.h
      TYPES_WITH_NAMESPACE xAOD::PhotonContainer xAOD::PhotonAuxContainer
                           xAOD::ElectronContainer xAOD::ElectronAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODEgamma TrigSerializeCnvSvcLib )
endif()

# Install files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
