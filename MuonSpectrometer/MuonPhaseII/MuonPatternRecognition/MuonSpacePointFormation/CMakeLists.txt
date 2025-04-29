# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePointFormation
################################################################################

# Declare the package name:
atlas_subdir( MuonSpacePointFormation )

atlas_add_component( MuonSpacePointFormation
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib AthenaBaseComps GaudiKernel MuonReadoutGeometryR4
                                    xAODMuonPrepData CxxUtils MuonSpacePoint xAODMuonViews MuonAlignmentDataR4)


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
