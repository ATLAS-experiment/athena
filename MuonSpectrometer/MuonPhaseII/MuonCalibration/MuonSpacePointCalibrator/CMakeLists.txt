# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePointCalibrator
################################################################################

# Declare the package name:
atlas_subdir( MuonSpacePointCalibrator )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead GeoModelHelpers )


atlas_add_component( MuonSpacePointCalibrator
                     src/components/*.cxx  src/*.cxx                 
                     LINK_LIBRARIES AthenaKernel GaudiKernel AthenaBaseComps StoreGateLib MuonRecToolInterfacesR4
                                    MdtCalibInterfacesLib MuonReadoutGeometryR4 MuonSpacePoint xAODMuonPrepData ActsCalibrationLib
                                    ActsGeometryInterfacesLib MdtCalibData)
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )