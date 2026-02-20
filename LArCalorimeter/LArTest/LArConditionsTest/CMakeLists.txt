# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArConditionsTest )

# Component(s) in the package:
atlas_add_component( LArConditionsTest
                     src/components/*_entries.cxx
                     src/LArCondDataTest.cxx
                     src/LArConditionsTestAlg.cxx
                     src/LArCablingTest.cxx
                     src/LArSCIdVsIdTest.cxx
                     LINK_LIBRARIES AthenaBaseComps CxxUtils StoreGateLib Identifier GaudiKernel LArCablingLib LArElecCalib LArIdentifier LArRawConditions LArRecConditions CaloDetDescrLib CaloGeoHelpers CaloIdentifier AthenaKernel  )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( LArConditionsTestWriteNoReg
		SCRIPT  python -m LArConditionsTest.LArConditionsTestConfig LArCondTest.Step=1
 		PROPERTIES TIMEOUT 1200
 		LOG_SELECT_PATTERN "DEBUG|ERROR" )

atlas_add_test( LArConditionsTestReadNoReg
		SCRIPT  python -m LArConditionsTest.LArConditionsTestConfig LArCondTest.Step=2
 		PROPERTIES TIMEOUT 1200
 		LOG_SELECT_PATTERN "DEBUG|ERROR"
	    	DEPENDS LArConditionsTestWriteNoReg)

atlas_add_test( LArConditionsTestWriteNoReg_RNTuple
		SCRIPT  python -m LArConditionsTest.LArConditionsTestConfig LArCondTest.Step=1 LArCondTest.FileName="LarCondTestNoReg_RNTuple.root" PoolSvc.DefaultContainerType="ROOTRNTUPLE"
		PROPERTIES TIMEOUT 1200
		LOG_SELECT_PATTERN "DEBUG|ERROR" )

atlas_add_test( LArConditionsTestReadNoReg_RNTuple
		SCRIPT  python -m LArConditionsTest.LArConditionsTestConfig LArCondTest.Step=2 LArCondTest.FileName="LarCondTestNoReg_RNTuple.root"
		PROPERTIES TIMEOUT 1200
		LOG_SELECT_PATTERN "DEBUG|ERROR"
	    DEPENDS LArConditionsTestWriteNoReg_RNTuple)

atlas_add_test( LArSCidVsIdTest 
	        SCRIPT python -m LArConditionsTest.LArSCidTest 
		POST_EXEC_SCRIPT noerror.sh )
