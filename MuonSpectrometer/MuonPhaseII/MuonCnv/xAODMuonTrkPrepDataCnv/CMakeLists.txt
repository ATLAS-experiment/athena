# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonTrkPrepDataCnv )

# External dependencies:
find_package( xAODUtilities )

# Component(s) in the package:
atlas_add_component( xAODMuonTrkPrepDataCnv
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     MuonPrepRawData xAODMuonPrepData StoreGateLib)

atlas_install_python_modules( python/*.py)

atlas_add_test(xAODMuonTrkPrepDataCnv_PRDtest
    SCRIPT test/testPRDConversion.py
    PROPERTIES TIMEOUT 900
    POST_EXEC_SCRIPT noerror.sh)
