# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

# User-defined cppcheck command line options:
set( ATLAS_CPPCHECK_OPTIONS "--enable=warning,portability,performance"
   CACHE STRING "cppcheck user-defined command line options" )

# Default options:
set( CMAKE_CPPCHECK_DEFAULT
   ${ATLAS_CPPCHECK_OPTIONS}
   "--quiet" "--inline-suppr" "--template=gcc"
   # allow conditionalizing code on cppcheck
   "-D__CPPCHECK__"
   # Lock-free atomic pointers (required by CxxUtils/CachedPointer.h)
   "-DATOMIC_POINTER_LOCK_FREE=2"
   # Commonly used libraries
   "--library=boost"
   "--library=googletest"
   "--library=python"
   # Athena-specific config and suppression file
   "--library=${_baseDir}/cppcheck_athena.cfg"
   "--suppressions-list=${_baseDir}/cppcheck_suppress.txt"
   CACHE STRING "cppcheck command line options" FORCE )
