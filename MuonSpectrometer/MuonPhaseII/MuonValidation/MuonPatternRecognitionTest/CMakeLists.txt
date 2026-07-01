################################################################################
# Package: MuonPatternRecognitionTest
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionTest )


find_package( ROOT COMPONENTS Gpad Graf Core Hist )

atlas_add_component( MuonPatternRecognitionTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib MuonPRDTestLib FourMomUtils 
                                    MuonTruthHelpers xAODMuon xAODMuonSimHit xAODMuonPrepData MuonVisualizationHelpersR4 
                                    MuonPatternEvent MuonPatternHelpers MuonReadoutGeometryR4 MuonPRDTestR4Lib ActsGeometryInterfacesLib
                                    ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_scripts( scripts/*.sh )

atlas_add_test( HoughTransformTestR3
                SCRIPT MuonPatternRecognitionTest_HoughTransformTestR3.sh
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( HoughTransformTestR3_NSWOnly
                SCRIPT MuonPatternRecognitionTest_HoughTransformTestR3_NSWOnly.sh
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)
                
atlas_add_test( HoughTransformTestR4
                SCRIPT MuonPatternRecognitionTest_HoughTransformTestR4.sh
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( DigiTester
                SCRIPT MuonPatternRecognitionTest_DigiTester.sh
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)
