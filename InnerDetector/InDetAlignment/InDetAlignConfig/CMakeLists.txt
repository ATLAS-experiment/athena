################################################################################
# Package: InDetAlignConfig
################################################################################

# Declare the package name:
atlas_subdir( InDetAlignConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( share/test_CL*.sh )
atlas_install_scripts( scripts/runIDAlign.py )

if (NOT SIMULATIONBASE)
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_CL" )

    atlas_add_test( InDetAlign_CL_L11
        SCRIPT test_CL_L11.sh 10
        PROPERTIES TIMEOUT 3600
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_CL
        POST_EXEC_SCRIPT noerror.sh)
       
    atlas_add_test( InDetAlign_CL_L16
        SCRIPT test_CL_L16.sh 10
        PROPERTIES TIMEOUT 3600
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_CL
        DEPENDS InDetAlign_CL_L11
        POST_EXEC_SCRIPT noerror.sh)
endif()
