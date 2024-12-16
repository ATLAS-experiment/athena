################################################################################
# Package: MuonCalibrationTest
################################################################################

# Declare the package name:
atlas_subdir( MuonCalibrationTest )

find_package( ROOT COMPONENTS Gpad Graf Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint Graf3d Html Postscript Gui GX11TTF GX11 )

atlas_add_component( MuonCalibrationTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib FourMomUtils MdtCalibInterfacesLib MuonIdHelpersLib
                                    xAODMuonSimHit xAODMuonPrepData MuonCalibEvent MuonReadoutGeometryR4 MdtCalibData ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

#atlas_add_test( MdtCalibrationTest
#                SCRIPT python -m MuonCalibrationTest.MuonCalibrationTesterConfig --nEvents 2 --noMM --noSTGC
#                PROPERTIES TIMEOUT 600
#                PRIVATE_WORKING_DIRECTORY
#                POST_EXEC_SCRIPT nopost.sh)