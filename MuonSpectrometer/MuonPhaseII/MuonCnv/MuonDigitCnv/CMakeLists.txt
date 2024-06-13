# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonDigitCnv )

# External dependencies:
find_package( xAODUtilities )

# Component(s) in the package:
atlas_add_component( MuonDigitCnv
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES  AthenaBaseComps GeoPrimitives MuonIdHelpersLib  
                                     MuonSimEvent xAODMuonSimHit StoreGateLib MdtCalibSvcLib
                                     xAODMuonPrepData MuonCondData GaudiKernel MuonDigitContainer)

#atlas_install_python_modules( python/*.py)