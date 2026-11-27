# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: ActsMuonDetector
################################################################################

# Declare the package name:
atlas_subdir( ActsMuonDetector )
find_package(Acts CONFIG COMPONENTS PluginGeoModel PluginJson)
find_package( GeoModelTools )

atlas_add_component( ActsMuonDetector
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel GeoModelUtilities ActsGeometryLib PathResolver MuonIdHelpersLib 
                                    ActsGeoUtils GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib 
                                    AthenaPoolUtilities Acts::PluginGeoModel Acts::PluginJson 
                                    GeoModelTools::GeoModelValidation)

# Install files from the package:
