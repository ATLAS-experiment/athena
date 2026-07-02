# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODL0MuonCand )
                         
# External dependencies.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODL0MuonCand
    xAODL0MuonCand/*.h xAODL0MuonCand/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODL0MuonCand
    LINK_LIBRARIES AthContainers AthLinks xAODBase xAODCore xAODMuonPrepData CxxUtils)

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODL0MuonCand/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::RPCCandDataContainer_v1" "xAOD::TGCCandDataContainer_v1" "xAOD::MDTCandDataContainer_v1" "xAOD::NSWCandDataContainer_v1" )

atlas_add_dictionary( xAODL0MuonCandDict
    xAODL0MuonCand/xAODL0MuonCandDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODL0MuonCand
    EXTRA_FILES Root/dict/*.cxx )
