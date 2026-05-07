# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigL0Muon )
                         
# External dependencies.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigL0Muon
    xAODTrigL0Muon/*.h xAODTrigL0Muon/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODTrigL0Muon
    LINK_LIBRARIES AthContainers AthLinks xAODBase xAODCore TrigT1MuctpiBits )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigL0Muon/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::SectorLogicCandDataContainer_v1" )

atlas_add_dictionary( xAODTrigL0MuonDict
    xAODTrigL0Muon/xAODTrigL0MuonDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODTrigL0Muon
    EXTRA_FILES Root/dict/*.cxx )
