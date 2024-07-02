# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsMuonDetector
################################################################################

# Declare the package name:
atlas_subdir( ActsMuonDetector )

atlas_add_component( ActsMuonDetector
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel GeoModelUtilities ActsGeometryLib MuonIdHelpersLib
                                    GaudiKernel MuonReadoutGeometryR4 AthenaPoolUtilities)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
