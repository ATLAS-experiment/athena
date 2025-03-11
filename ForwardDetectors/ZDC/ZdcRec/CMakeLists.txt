# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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
atlas_add_test( ZDCCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data23_hi.00463427.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCCalibTest24
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data24_hi.00488915.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCLEDCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data23_hi.00463427.calibration_ZDCLEDCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
  POST_EXEC_SCRIPT noerror.sh)
atlas_add_test( ZDCInjCalibTest
  SCRIPT python -m ZdcRec.ZdcRecConfig --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data24_hicomm.00488824.calibration_ZDCInjCalib.daq.RAW._lb0000._SFO-11._0001.data --evtMax=10
  POST_EXEC_SCRIPT noerror.sh)
		     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
