# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePointCalibrator
################################################################################

# Declare the package name:
atlas_subdir( MuonSpacePointCalibrator )

find_package( Acts COMPONENTS Core )

atlas_add_component( MuonSpacePointCalibrator
                     src/components/*.cxx  src/*.cxx                 
                     LINK_LIBRARIES AthenaKernel GaudiKernel AthenaBaseComps StoreGateLib MuonRecToolInterfacesR4
                                    MdtCalibInterfacesLib MuonReadoutGeometryR4 MuonSpacePoint xAODMuonPrepData ActsCalibratorsLib
                                    ActsGeometryInterfacesLib MdtCalibData MuonPatternHelpers NSWCalibToolsLib MMClusterizationLib
                                    Acts::Core)
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )