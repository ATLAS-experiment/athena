xAODMuonInference# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODMuonInference )

# Pull in the helper CMake code.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODMuonInference
   xAODMuonInference/*.h xAODMuonInference/versions/*.h Root/*.cxx
   PUBLIC_HEADERS xAODMuonInference
   LINK_LIBRARIES AthContainers xAODCore )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODMuonInference/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::MuonDVInferenceResultContainer_v1"
   OBJECTS "xAOD::MuonDVInferenceResult_v1" )

atlas_add_dictionary( xAODMuonInferenceDict
   xAODMuonInference/xAODMuonInferenceDict.h
   ${_selectionFile}
   LINK_LIBRARIES xAODCore xAODMuonInference
   EXTRA_FILES Root/dict/*.cxx )
