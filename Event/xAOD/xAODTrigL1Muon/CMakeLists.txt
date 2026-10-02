# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigL1Muon )

# External dependencies.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigL1Muon
    xAODTrigL1Muon/*.h xAODTrigL1Muon/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODTrigL1Muon
    LINK_LIBRARIES AthContainers AthLinks xAODBase xAODCore TrigT1MuctpiBits )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigL1Muon/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::SectorLogicCandDataContainer_v1" )

atlas_add_dictionary( xAODTrigL1MuonDict
    xAODTrigL1Muon/xAODTrigL1MuonDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODTrigL1Muon
    EXTRA_FILES Root/dict/*.cxx )
