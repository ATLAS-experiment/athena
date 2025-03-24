# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonSPId )
find_package( onnxruntime )


# Component(s) in the package:
atlas_add_component( MuonSPId
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     MuonSimEvent xAODMuonSimHit StoreGateLib MdtCalibSvcLib
                                     xAODMuonPrepData MuonSpacePoint xAODMuon MuonTesterTreeLib
                                     MuonPatternEvent MuonPatternHelpers MuonPatternHelpers 
                                     MuonInferenceInterfaces
                                     ${ONNXRUNTIME_LIBRARIES}
                                     )
                                     
atlas_install_python_modules( python/*.py)

atlas_add_test( testMuonSPId
	SCRIPT python -m MuonSPId.muonSPIdDump --nEvents 5 --noSTGC --noMM
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

# atlas_install_data( models/* )
