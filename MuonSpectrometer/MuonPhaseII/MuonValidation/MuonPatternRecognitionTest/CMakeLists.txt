################################################################################
# Package: MuonPatternRecognitionTest
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionTest )


find_package( ROOT COMPONENTS Gpad Graf Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint Graf3d Html Postscript Gui GX11TTF GX11 )

atlas_add_component( MuonPatternRecognitionTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib 
                                    xAODMuonSimHit xAODMuonPrepData MuonPatternEvent MuonReadoutGeometryR4 ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
