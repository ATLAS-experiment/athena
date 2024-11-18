# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Additional CMake settings for the build. Used by Projects/.
#
# Sets:
#   ATLAS_GCC_CHECKERS_CONFIG
#
# Can be steered by:
#   CHECKERGCCPLUGINS_PEDANTIC - enable pedantic checkergcc settings
#

# Setup compiler settings for the checkergcc plugins:

# Resolve relative path to make it look "nicer":
get_filename_component( _baseDir
   "${CxxUtilsSettings_DIR}/../share" ABSOLUTE )

# Default configuration file:
set( _config "${_baseDir}/checkergcc-base.config" )

# Append build specific configuration file:
if( CHECKERGCCPLUGINS_PEDANTIC )
   set( _config "${_config}:${_baseDir}/checkergcc-pedantic.config" )
else()
   set( _config "${_config}:${_baseDir}/checkergcc.config" )
endif()

# Configure the checker:
set( ATLAS_GCC_CHECKERS_CONFIG ${_config}
   CACHE STRING "Configuration file(s) for the GCC checker plugins" FORCE )

# CppCheck options:
option( ATLAS_USE_CPPCHECK "Use CppCheck in the build" ON )

# User-defined cppcheck command line options:
set( ATLAS_CPPCHECK_OPTIONS "--enable=warning,portability,performance"
   CACHE STRING "cppcheck user-defined command line options" )

# Default options:
set( CMAKE_CPPCHECK_DEFAULT
   ${ATLAS_CPPCHECK_OPTIONS}
   "--quiet" "--inline-suppr" "--template=gcc"
   # allow conditionalizing code on cppcheck
   "-D__CPPCHECK__"
   # Commonly used libraries
   "--library=boost"
   "--library=googletest"
   "--library=posix"
   "--library=python"
   "--library=qt"
   CACHE STRING "cppcheck command line options" FORCE )

# Athena-specific config and suppression file:
list( APPEND CMAKE_CPPCHECK_DEFAULT "--library=${_baseDir}/cppcheck_athena.cfg" )
list( APPEND CMAKE_CPPCHECK_DEFAULT "--suppressions-list=${_baseDir}/cppcheck_suppress.txt" )
