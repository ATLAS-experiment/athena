# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: MuonPatternRecognitionAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionAlgs )




atlas_add_component( MuonPatternRecognitionAlgs
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES MuonVisualizationHelpersR4 MuonPatternEvent AthenaKernel StoreGateLib
                                    xAODMuonPrepData MuonSpacePoint MuonPatternHelpers MuonRecToolInterfacesR4 
                                    MuonTruthHelpers TruthUtils ActsToolInterfacesLib MuonTrackEvent ActsCalibratorsLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh )
