################################################################################
# Package: TgcRawDataMonitoring
################################################################################

# Declare the package name:
atlas_subdir( TgcRawDataMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore )

atlas_add_component( TgcRawDataMonitoring
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel AthenaMonitoringLib
                     StoreGateLib MuonReadoutGeometry MuonIdHelpersLib MuonPrepRawData MuonTrigCoinData MuonDQAUtilsLib Identifier xAODEventInfo xAODTracking xAODMuon xAODTrigger MuonRDO AnalysisTriggerEvent muonEvent TrigSteeringEvent TrkExInterfaces FourMomUtils MuonAnalysisInterfacesLib GoodRunsListsLib CxxUtils )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

