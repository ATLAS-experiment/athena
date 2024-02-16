# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArConditionsTest )

# Component(s) in the package:
atlas_add_component( LArConditionsTest
                     src/LArConditionsTest_entries.cxx
                     src/LArCondDataTest.cxx
                     src/LArConditionsTestAlg.cxx
                     src/LArCablingTest.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils StoreGateLib Identifier GaudiKernel LArCablingLib LArElecCalib LArIdentifier LArRawConditions LArRawUtilsLib LArRecConditions CaloDetDescrLib CaloEvent CaloIdentifier AthenaKernel CaloInterfaceLib CaloConditions )

# Install files from the package:
atlas_install_joboptions( share/*.py )

# Tests in the package:
function (larconditions_run_test testName)
  cmake_parse_arguments( ARG "" "DEPENDS" "" ${ARGN} )

  atlas_add_test( ${testName}
                  SCRIPT athena.py LArConditionsTest/${testName}_jobOptions.py
                  ENVIRONMENT ATLAS_REFERENCE_TAG=LArConditionsTest/LArConditionsTest-01-00-12
                  PROPERTIES TIMEOUT 1200
 		  LOG_SELECT_PATTERN "DEBUG|ERROR"
                  DEPENDS ${ARG_DEPENDS} )

  # Turn off POOL verbose printing:
  set_property(TEST LArConditionsTest_${testName}_ctest
     APPEND PROPERTY ENVIRONMENT POOL_OUTMSG_LEVEL=4)

endfunction (larconditions_run_test)

larconditions_run_test (LArConditionsTest)
larconditions_run_test (LArConditionsTestWriteNoReg)
larconditions_run_test (LArConditionsTestReadNoReg DEPENDS LArConditionsTestWriteNoReg)
