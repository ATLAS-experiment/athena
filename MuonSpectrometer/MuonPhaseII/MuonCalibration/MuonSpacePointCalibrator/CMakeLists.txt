# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonSpacePointCalibrator
################################################################################

# Declare the package name:
atlas_subdir( MuonSpacePointCalibrator )

# External dependencies:
find_package( GeoModel COMPONENTS GeoModelKernel GeoModelDBManager GeoModelRead GeoModelHelpers )

# Component(s) in the package:
atlas_add_library( MuonSpacePointCalibratorLib
                   MuonSpacePointCalibrator/*.h 
                   INTERFACE
                   PUBLIC_HEADERS MuonSpacePointCalibrator 
                   LINK_LIBRARIES  GaudiKernel AthenaBaseComps MuonSpacePoint)

atlas_add_component( MuonSpacePointCalibrator
                     src/components/*.cxx  src/*.cxx                 
                     LINK_LIBRARIES AthenaKernel GaudiKernel AthenaBaseComps StoreGateLib 
                                    MuonSpacePointCalibratorLib MdtCalibInterfacesLib MuonReadoutGeometryR4
                                    MuonSpacePoint xAODMuonPrepData ActsGeometryInterfacesLib MuonDigitContainer)
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )