# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( LArCalibTools )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Component(s) in the package:
atlas_add_component( LArCalibTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AthenaPoolUtilities CaloCondBlobObjs CaloDetDescrLib CaloIdentifier CxxUtils GaudiKernel LArCOOLConditions LArCablingLib LArElecCalib LArIdentifier LArRawConditions LArRawEvent LArRecConditions LumiBlockData StoreGateLib TrigDecisionToolLib xAODEventInfo TrigT1CaloEventLib TrigT1CaloCalibToolInterfaces TrigT1CaloToolInterfaces TrigT1Interfaces )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
atlas_install_scripts( share/LArConditions2Ntuple.py )


atlas_add_test( LArConditions2Ntuple
		SCRIPT LArConditions2Ntuple.py 
                POST_EXEC_SCRIPT noerror.sh )
