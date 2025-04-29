################################################################################
# Package: MuonCalibrationTest
################################################################################

# Declare the package name:
atlas_subdir( MuonCalibrationTest )

find_package( ROOT COMPONENTS Gpad Core Hist )

atlas_add_component( MuonCalibrationTest
                     src/components/*.cxx src/*.cxx 
                     INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib FourMomUtils MdtCalibInterfacesLib MuonIdHelpersLib
                                    xAODMuonSimHit xAODMuonPrepData MuonCalibEvent MuonReadoutGeometryR4 MdtCalibData ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( MdtCalibrationTest
                SCRIPT python -m MuonCalibrationTest.MuonCalibrationTesterConfig --nEvents 2
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)