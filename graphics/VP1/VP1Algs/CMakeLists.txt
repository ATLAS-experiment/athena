# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1Algs )

# "Externals" dependencies - They are needed at runtime, only
find_package( libxkbcommon ) 
find_package( glib ) 
find_package( libffi )

# Component(s) in the package:
atlas_add_component( VP1Algs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel PathResolver xAODEventInfo 
                     PRIVATE_LINK_LIBRARIES VP1Gui VP1UtilsBase TrkExInterfaces )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/vp1 )
atlas_install_generic( share/offscreen-workaround/*
   DESTINATION share/VP1Algs/offscreen-workaround )
atlas_install_scripts( share/clear-bash-hash-table-vp1 )

#----------------------------------------------------------
# CONFIGURING THE ENVIRONMENT VARIABLES FOR THE PACKAGE   #
#
#message( "CMAKE_BINARY_DIR path:" ${CMAKE_BINARY_DIR} )
#message( "ATLAS_PLATFORM path:" ${ATLAS_PLATFORM} )
# set a variable to be used in the 'Config' script. 
#   The variable will be substituted with the path when file is 'configured' 
#   and copied from 'source' to 'binary'
set( BUILD_DIR ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM} )
# Configure the environment setup module. 
#   The 'configure_file' command here below copies the input file in the 'source' dir
#   to the output file in 'binary' dir, e.g. in ../build/graphics/VP1/VP1Algs/
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/VP1AlgsEnvironmentConfig.cmake.in  
    ${CMAKE_CURRENT_BINARY_DIR}/VP1AlgsEnvironmentConfig.cmake   
    @ONLY )
# Set the folder where CMake can find the 'Config' file for the dummy package
set( VP1AlgsEnvironment_DIR ${CMAKE_CURRENT_BINARY_DIR}  
    CACHE PATH "Location of VP1AlgsEnvironmentConfig.cmake" 
    FORCE 
    )
# And now "find" it! It will set the env vars defined in the package's 'Config' file.
find_package( VP1AlgsEnvironment )
# message( "VP1PLUGINPATH has been set to: " $ENV{VP1PLUGINPATH} )


# Check Phase-II defaults without starting geometry or the VP1 GUI.
atlas_add_test( VP1PhaseIIConfig
   SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/VP1PhaseIIConfig_test.py
   POST_EXEC_SCRIPT noerror.sh )

# Configuration regression tests do not start the VP1 GUI or event loop.
atlas_add_test( VP1AlgConfig
   SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/VP1AlgConfig_test.py
   POST_EXEC_SCRIPT noerror.sh )

# The snapshot workaround must activate before Athena imports or GL loading.
atlas_add_test( VP1SnapshotWorkaround
   SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/VP1SnapshotWorkaround_test.py
   POST_EXEC_SCRIPT noerror.sh )

# Exercise private-handle and RTLD_DEFAULT GLX lookups without an X server.
atlas_add_test( VP1GLXShim
   SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/VP1GLXShim_test.py
   POST_EXEC_SCRIPT noerror.sh )
