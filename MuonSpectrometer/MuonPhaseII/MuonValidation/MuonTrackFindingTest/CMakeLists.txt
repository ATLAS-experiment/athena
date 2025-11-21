################################################################################
# Package: MuonPatternRecognitionTest
################################################################################

# Declare the package name:
atlas_subdir( MuonTrackFindingTest )


find_package( ROOT COMPONENTS Gpad Graf Core Hist )

atlas_add_component( MuonTrackFindingTest
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


atlas_add_test( MsTrackFinderTestR3
                SCRIPT python -m MuonTrackFindingTest.MsTrackFindingTester --nEvents 1 --noPerfMon
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 1200)

atlas_add_test( SegmentRefitTestR4
                SCRIPT python -m MuonTrackFindingTest.SegmentRefitTester --nEvents 1 --noPerfMon
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 300)
