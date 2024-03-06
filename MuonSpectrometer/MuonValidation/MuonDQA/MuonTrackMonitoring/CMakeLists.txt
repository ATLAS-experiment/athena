################################################################################
# Package: MuonTrackMonitoring
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Graf Graf3d Gpad Html Postscript Gui GX11TTF GX11 )

# Component(s) in the package:
atlas_add_component( MuonTrackMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaMonitoringLib StoreGateLib GeoPrimitives EventPrimitives FourMomUtils xAODEventInfo 
                                    xAODMuon xAODTracking GaudiKernel MuonAnalysisInterfacesLib MuonPrepRawData MuonRecHelperToolsLib 
                                    MuonHistUtils TrkEventPrimitives TrkParameters TrkTrack TrkToolInterfaces 
                                    TrkValHistUtils TrigConfL1Data TrkMeasurementBase )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
