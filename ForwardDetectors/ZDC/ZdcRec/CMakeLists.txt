# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcRec )

# External dependencies:
find_package( GSL )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( ZdcRecLib
                   src/*.cxx
                   PUBLIC_HEADERS ZdcRec
                   INCLUDE_DIRS ${GSL_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GSL_LIBRARIES} AsgTools AthenaBaseComps xAODForward xAODTrigL1Calo ZdcEvent CaloSimEvent GaudiKernel StoreGateLib ZdcAnalysisLib ZdcByteStreamLib ZdcIdentifier ZdcUtilsLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} CxxUtils ZdcByteStreamLib ZdcConditions )

atlas_add_component( ZdcRec
                     src/components/*.cxx
                     LINK_LIBRARIES ZdcRecLib )

# Disabled in main as the test do not run
# atlas_add_test( ZDCCalibTest15
#   SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data15_hi.00287321.calibration_zdcCalib.daq.RAW._lb0000._SFO-2._0001.data --evtMax=10
#   POST_EXEC_SCRIPT noerror.sh)
# atlas_add_test( ZDCCalibTest16
#   SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data16_hip8TeV.00313063.calibration_zdcCalib.daq.RAW._lb0000._SFO-3._0001.data --evtMax=10
#   POST_EXEC_SCRIPT noerror.sh)
# atlas_add_test( ZDCCalibTest18
#   SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data18_hi.00367365.calibration_zdcCalib.daq.RAW._lb0000._SFO-5._0002.data --evtMax=10
#   POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data23_hi.00463427.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  PRIVATE_WORKING_DIRECTORY
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCCalibTest24
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data24_hi.00488915.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCLEDCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data23_hi.00463427.calibration_ZDCLEDCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  PRIVATE_WORKING_DIRECTORY
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCInjCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data24_hicomm.00488824.calibration_ZDCInjCalib.daq.RAW._lb0000._SFO-11._0001.data --evtMax=10
  PRIVATE_WORKING_DIRECTORY
  POST_EXEC_SCRIPT noerror.sh)
		     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
