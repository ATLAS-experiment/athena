# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonFastRecoAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonFastRecoAlgs )
find_package( Acts COMPONENTS Core )

atlas_add_component( MuonFastRecoAlgs
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES MuonVisualizationHelpersR4 AthenaKernel StoreGateLib xAODMuon 
                                    xAODMuonPrepData MuonSpacePoint MuonRecToolInterfacesR4 xAODMuonViews 
                                    ActsToolInterfacesLib MuonDetDescrUtils
                                    ActsEventLib MuonFastRecoEvent MuonFastRecoHelpers)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )