################################################################################
# Package: InDetAlignConfig
################################################################################

# Declare the package name:
atlas_subdir( InDetAlignConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_scripts( scripts/runIDAlign.py )
atlas_install_scripts( scripts/runITkAlign.py )


if (NOT SIMULATIONBASE)
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_inDet" )
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_silicon" )
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_pixel_only" )
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_sct_only" )
    file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_trt_only" )

    atlas_add_test( InDetAlign_accumulate_pixel_only
        SCRIPT runIDAlign.py --maxEvents 10 -a --alignPixel
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_pixel_only
        POST_EXEC_SCRIPT noerror.sh)
        
    atlas_add_test( InDetAlign_solve_pixel_only
        SCRIPT runIDAlign.py -s --alignPixel
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_pixel_only
        DEPENDS InDetAlign_accumulate_pixel_only
        POST_EXEC_SCRIPT noerror.sh)
        
    atlas_add_test( InDetAlign_accumulate_sct_only
        SCRIPT runIDAlign.py --maxEvents 10 -a --alignSCT
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_sct_only
        POST_EXEC_SCRIPT noerror.sh)
        
    atlas_add_test( InDetAlign_solve_sct_only
        SCRIPT runIDAlign.py -s --alignSCT
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_sct_only
        DEPENDS InDetAlign_accumulate_sct_only
        POST_EXEC_SCRIPT noerror.sh)
        
  #  atlas_add_test( InDetAlign_accumulate_trt_only
  #      SCRIPT runIDAlign.py --maxEvents 10 -a --alignTRT
  #      PROPERTIES TIMEOUT 1800
  #      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_trt_only
   #     POST_EXEC_SCRIPT noerror.sh)
        
  #  atlas_add_test( InDetAlign_solve_trt_only
  #      SCRIPT runIDAlign.py -s --alignTRT
  #      PROPERTIES TIMEOUT 1800
  #      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_trt_only
  #      DEPENDS InDetAlign_accumulate_trt_only
  #      POST_EXEC_SCRIPT noerror.sh)
        
    atlas_add_test( InDetAlign_accumulate_silicon
        SCRIPT runIDAlign.py --maxEvents 10 -a --alignSilicon
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_silicon
        POST_EXEC_SCRIPT noerror.sh)
        
    atlas_add_test( InDetAlign_solve_silicon
        SCRIPT runIDAlign.py -s --alignSilicon
        PROPERTIES TIMEOUT 1800
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_silicon
        DEPENDS InDetAlign_accumulate_silicon
        POST_EXEC_SCRIPT noerror.sh)
        
  #  atlas_add_test( InDetAlign_accumulate_inDet
 #       SCRIPT runIDAlign.py --maxEvents 10 -a --alignInDet
 #       PROPERTIES TIMEOUT 1800
 #       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_inDet
 #       POST_EXEC_SCRIPT noerror.sh)
        
  #  atlas_add_test( InDetAlign_solve_inDet
 #       SCRIPT runIDAlign.py -s --alignInDet
 #       PROPERTIES TIMEOUT 1800
 #       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unitTestRun_inDet
 #       DEPENDS InDetAlign_accumulate_inDet
 #       POST_EXEC_SCRIPT noerror.sh)
endif()
