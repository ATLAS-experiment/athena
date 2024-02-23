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

