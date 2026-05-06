# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthenaPoolTest )

# Component(s) in the package:
atlas_add_component( AthenaPoolTest
                     src/AthenaPoolTestDataReader.cxx
                     src/AthenaPoolTestDataWriter.cxx
                     src/InDetRawDataFakeReader.cxx
                     src/InDetRawDataFakeWriter.cxx
                     src/LArCellContFakeWriter.cxx
                     src/LArCellContFakeReader.cxx
                     src/EventInfoWriter.cxx
                     src/EventInfoReader.cxx
                     src/TrkTrackFakeReader.cxx
                     src/TrkTrackFakeWriter.cxx
                     src/FakeTrackBuilder.cxx
                     src/RDOReaderDoubleSelector.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaPoolTestData CaloDetDescrLib CaloEvent CaloIdentifier AthenaBaseComps StoreGateLib CxxUtils GeoModelInterfaces GeoPrimitives Identifier GaudiKernel InDetIdentifier InDetReadoutGeometry InDetRawData InDetCompetingRIOsOnTrack InDetPrepRawData InDetRIO_OnTrack LArRecEvent TrkSurfaces TrkCompetingRIOsOnTrack TrkEventPrimitives TrkMaterialOnTrack TrkParameters TrkTrack TrkVertexOnTrack EventInfo EventInfoMgtLib xAODEventInfo )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
function (athenapooltest_run_test testName jo pattern)
  cmake_parse_arguments( ARG "PRIVATE_WORKING_DIRECTORY" "" "DEPENDS" ${ARGN} )
  set(_extraArgs)
  if( ARG_PRIVATE_WORKING_DIRECTORY )
     set(_extraArgs "PRIVATE_WORKING_DIRECTORY")
  endif()
  atlas_add_test( ${testName}
                  SCRIPT athena.py AthenaPoolTest/${jo}.py
                  ${_extraArgs}
                  LOG_SELECT_PATTERN ${pattern}
                  LOG_IGNORE_PATTERN "0x[0-9a-f]{4,}|DEBUG Try item:|AthenaAttribute.*DB=|AtlasRelease|Release version|data dependencies"
                  PROPERTIES TIMEOUT 900
                  DEPENDS ${ARG_DEPENDS} )
endfunction (athenapooltest_run_test)

function (athenapooltest_runca_test testName jo pattern)
  cmake_parse_arguments( ARG "PRIVATE_WORKING_DIRECTORY" "" "DEPENDS" ${ARGN} )
  set(_extraArgs)
  if( ARG_PRIVATE_WORKING_DIRECTORY )
     set(_extraArgs "PRIVATE_WORKING_DIRECTORY")
  endif()
  atlas_add_test( ${testName}
                  SCRIPT python -m AthenaPoolTest.${jo}
                  ${_extraArgs}
                  LOG_SELECT_PATTERN ${pattern}
                  LOG_IGNORE_PATTERN "0x[0-9a-f]{4,}|DEBUG Try item:|AthenaAttribute.*DB=|AtlasRelease|Release version|data dependencies"
                  PROPERTIES TIMEOUT 900
                  DEPENDS ${ARG_DEPENDS} )
endfunction (athenapooltest_runca_test)



# Write/read out some simple objects:
athenapooltest_runca_test( AthenaPoolTestWrite AthenaPoolTestWrite
                           "^AthenaPoolTestD" )

athenapooltest_runca_test( AthenaPoolTestRead AthenaPoolTestRead
                           "AthenaPoolTestD"
                           DEPENDS AthenaPoolTestWrite )

# normal EventSelector
athenapooltest_runca_test( AthenaPoolTestStep2Write AthenaPoolTestStep2Write
                           "^AthenaPoolTestD"
                           DEPENDS AthenaPoolTestWrite )

# Test write/read of indet rdos:
athenapooltest_runca_test( InDetRawDataWriter InDetRawDataWriter
                           "InDetRawDataFakeWriter" )

athenapooltest_runca_test( InDetRawDataReader InDetRawDataReader
                           "InDetRawDataFakeReader"
                           DEPENDS InDetRawDataWriter )

# Check for adding DetDescr tags to event info object:
athenapooltest_runca_test( EventInfoWrite EventInfoWrite
                         "^TagInfoMgr|^EventInfoWriter" )
athenapooltest_runca_test( EventInfoWriteStep2 EventInfoWriteStep2
                         "^TagInfoMgr|^EventInfoWriter"
                         DEPENDS EventInfoWrite )
athenapooltest_runca_test( EventInfoWriteStep3 EventInfoWriteStep3
                         "^TagInfoMgr|^EventInfoWriter"
                         DEPENDS EventInfoWriteStep2 )
athenapooltest_runca_test( EventInfoWriteStep4 EventInfoWriteStep4
                         "^TagInfoMgr|^EventInfoWriter"
                         DEPENDS EventInfoWriteStep3 )
athenapooltest_runca_test( EventInfoWriteStep5 EventInfoWriteStep5
                         "^TagInfoMgr|^EventInfoWriter"
                         DEPENDS EventInfoWriteStep4 )
athenapooltest_runca_test( EventInfoRead EventInfoRead
                         "^TagInfoMgr|^EventInfoWriter"
                         DEPENDS EventInfoWriteStep5 )
