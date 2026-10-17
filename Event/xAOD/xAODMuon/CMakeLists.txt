# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODMuon )

# External dependencies.
find_package( ROOT COMPONENTS Core GenVector )
find_package( xAODUtilities )

# Extra dependencies, based on the build environment.
set( extra_libs )
if( NOT XAOD_ANALYSIS AND NOT GENERATIONBASE )
   set( extra_libs TrkSegment )
endif()

# Component(s) in the package.
atlas_add_library( xAODMuon
   xAODMuon/*.h xAODMuon/versions/*.h Root/*.cxx
   PUBLIC_HEADERS xAODMuon
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthLinks xAODBase
                 xAODCaloEvent xAODCore xAODPrimitives xAODTracking MuonStationIndexLib
                 ${extra_libs}
   PRIVATE_LINK_LIBRARIES TruthUtils )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODMuon/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::MuonContainer_v1" "xAOD::MuonSegmentContainer_v1"
              "xAOD::SlowMuonContainer_v1" )

atlas_add_dictionary( xAODMuonDict
   xAODMuon/xAODMuonDict.h
   ${_selectionFile}
   LINK_LIBRARIES xAODCore xAODMuon
   EXTRA_FILES Root/dict/*.cxx )

atlas_add_dictionary( xAODMuonEnumsDict
   xAODMuon/xAODMuonEnumsDict.h
   xAODMuon/selectionEnums.xml)

# Serializer converter for Trigger.
if( NOT SIMULATIONBASE AND NOT GENERATIONBASE AND NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODMuonSerCnv
      FILES xAODMuon/MuonContainer.h xAODMuon/MuonAuxContainer.h
      TYPES_WITH_NAMESPACE xAOD::MuonContainer xAOD::MuonAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODMuon TrigSerializeCnvSvcLib )
endif()

# Install files from the package.
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})

# Test(s) in the package.
atlas_add_test( xAODMuon_Muon_test
   SOURCES test/xAODMuon_Muon_test.cxx
   LINK_LIBRARIES xAODMuon )

atlas_add_test( ut_xAODOMuonEnums_pytest
    SCRIPT test/ut_xAODMuonEnums_pytest.py )

