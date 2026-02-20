################################################################################
# Package: MuonReleaseTestsR4
################################################################################

# Declare the package name:
atlas_subdir( MuonReleaseTestsR4 )

atlas_install_scripts( test/*.sh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )

atlas_add_test( SpotIntegrationTest
                SCRIPT test_MuonR4Reco_spot.sh 1 3 --noPerfMon
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
