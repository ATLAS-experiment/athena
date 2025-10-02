################################################################################
# Package: MuonPatternRecognitionTest
################################################################################

# Declare the package name:
atlas_subdir( MuonSPCalibrationTest )


find_package( ROOT COMPONENTS Gpad Graf Core Hist )

atlas_add_component( MuonSPCalibrationTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib MuonPRDTestLib FourMomUtils 
                                    MuonTruthHelpers xAODMuon xAODMuonSimHit xAODMuonPrepData MuonVisualizationHelpersR4 
                                    MuonPatternEvent MuonPatternHelpers MuonTrackEvent MuonReadoutGeometryR4 MuonPRDTestR4Lib 
                                    ActsToolInterfacesLib ActsGeometryInterfacesLib MuonTrackFindingToolsLib MuonDetDescrUtils 
                                    ActsCalibratorsLib
                                    ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )