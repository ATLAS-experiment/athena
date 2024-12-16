################################################################################
# Package: MuonPatternRecognitionTest
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionTest )


find_package( ROOT COMPONENTS Gpad Graf Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint Graf3d Html Postscript Gui GX11TTF GX11 )

atlas_add_component( MuonPatternRecognitionTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib MuonPRDTestLib FourMomUtils 
                                    MuonTruthHelpers xAODMuon xAODMuonSimHit xAODMuonPrepData MuonVisualizationHelpersR4 
                                    MuonPatternEvent MuonPatternHelpers MuonReadoutGeometryR4 MuonPRDTestR4Lib ActsGeometryInterfacesLib
                                    ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( HoughTransformTestR3
                SCRIPT python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig --nEvents 2  --noMonitorPlots
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

#atlas_add_test( HoughTransformTestR4
#                SCRIPT python -m MuonPatternRecognitionTest.MuonHoughTransformTesterConfig --noMonitorPlots --nEvents 2 --noMM --noSTGC --geoModelFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R4-MUONTEST.db --condTag OFLCOND-MC21-SDR-RUN4-01 --geoTag ATLAS-P2-RUN4-01-00-00 --inputFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root
#                PROPERTIES TIMEOUT 600
#                PRIVATE_WORKING_DIRECTORY
#                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( FullChainIntegrationTest
                SCRIPT python -m MuonPatternRecognitionTest.MuonRecoChainTesterConfig --nEvents 2 --noMM --noSTGC
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
