################################################################################
# Package: MuonFastDigiTest
################################################################################

# Declare the package name:
atlas_subdir( MuonPRDTestR4 )

atlas_add_library( MuonPRDTestR4Lib
                   Root/*.cxx
                   PUBLIC_HEADERS MuonPRDTestR4
                   LINK_LIBRARIES MuonReadoutGeometryR4 GeoPrimitives Identifier ActsGeometryInterfacesLib 
                                  MuonIdHelpersLib MuonTesterTreeLib GeoModelUtilities xAODMuonSimHit MuonTrackEvent
                                  xAODMuonPrepData CxxUtils MuonSpacePoint ActsEventLib MuonPatternHelpers)

atlas_add_component( MuonPRDTestR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib MuonTesterTreeLib MuonPRDTestLib
                                    xAODMuonSimHit xAODMuonPrepData MuonPRDTestR4Lib MuonTruthHelpers)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


atlas_add_test( SimHitTester
                SCRIPT python -m MuonPRDTestR4.MuonHitTestConfig --nEvents 1 --noMM --noSTGC
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)


