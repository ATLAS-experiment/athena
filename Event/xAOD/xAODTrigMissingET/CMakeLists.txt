# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigMissingET )

# Pull in the helper CMake code.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigMissingET
   xAODTrigMissingET/versions/*.h xAODTrigMissingET/*.h Root/*.cxx
   PUBLIC_HEADERS xAODTrigMissingET
   LINK_LIBRARIES AthContainers xAODCore )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigMissingET/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::TrigMissingETContainer_v1" )

atlas_add_dictionary( xAODTrigMissingETDict
   xAODTrigMissingET/xAODTrigMissingETDict.h
   ${_selectionFile}
   LINK_LIBRARIES xAODCore xAODTrigMissingET
   EXTRA_FILES Root/dict/*.cxx )

if( NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODTrigMissingETxAODSerCnv
      FILES xAODTrigMissingET/TrigMissingETContainer.h xAODTrigMissingET/TrigMissingETAuxContainer.h
      TYPES_WITH_NAMESPACE xAOD::TrigMissingETContainer xAOD::TrigMissingETAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODTrigMissingET TrigSerializeCnvSvcLib )
endif()
