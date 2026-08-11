# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigValAlgs )

# Component(s) in the package:
atlas_add_component( TrigValAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthAnalysisBaseCompsLib AthViews AthenaBaseComps GaudiKernel MuonCombinedToolInterfaces StoreGateLib TrigCompositeUtilsLib TrigConfData TrigConfHLTData TrigConfHLTUtilsLib TrigDecisionToolLib TrigNavStructure TrigRoiConversionLib TrigSteeringEvent TrigT1Interfaces xAODBTagging xAODCore xAODEgamma xAODJet xAODMuon xAODTau xAODTracking xAODTrigBphys xAODTrigCalo xAODTrigEgamma xAODTrigMinBias xAODTrigMissingET xAODTrigMuon xAODTrigger xAODEventInfo )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/dumpTriggerInfo.py
                       share/generateTrigNavGraph.py
                       share/trigEDMChecker.py
                       POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( TrigEDMChecker
                SCRIPT python -m TrigValAlgs.TrigValAlgsConfig
                POST_EXEC_SCRIPT noerror.sh )
