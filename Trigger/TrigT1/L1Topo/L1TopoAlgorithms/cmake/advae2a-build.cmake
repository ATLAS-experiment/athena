# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Recipe for building ADVAE2A as part of the package.
#

# CMake include(s).
include( ExternalProject )

# Declare where to get ADVAE2A from.
set( ATLAS_ADVAE2A_SOURCE
   "URL;http://cern.ch/atlas-software-dist-eos/externals/AnomDetVAE2A/l1topo2a-vae-0.4.0.tar.gz;https://gitlab.cern.ch/atlas-l1calo/l1topo/specialAlgorithms/anomaly-detection-vae/l1topo2a-vae/-/archive/0.4.0/l1topo2a-vae-0.4.0.tar.gz;URL_MD5;9c45f5f282102323e8b026f6e2dd6882"
   CACHE STRING "Source for the ADVAE2A project" )
set( ATLAS_ADVAE2A_PATCH ""
   CACHE STRING "Patch command for ADVAE2A" )
set( ATLAS_ADVAE2A_FORCEDOWNLOAD_MESSAGE
   "Forcing the re-download of ADVAE2A"
   CACHE STRING "Download message to update whenever patching changes" )
mark_as_advanced( ATLAS_ADVAE2A_SOURCE ATLAS_ADVAE2A_PATCH
   ATLAS_ADVAE2A_FORCEDOWNLOAD_MESSAGE )

# Files / directories produced by the following build.
set( ADVAE2A_INSTALL_DIR
   "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/advae2a-install" )
set( ADVAE2A_INCLUDE_DIRS
   "${ADVAE2A_INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}" )
set( ADVAE2A_LIBRARIES
   "${ADVAE2A_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}advae2a${CMAKE_STATIC_LIBRARY_SUFFIX}" )

# Build ADVAE2A into a static library, that would only be used privately
# by this package.
ExternalProject_Add( ADVAE2A
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   PREFIX "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
   INSTALL_DIR "${ADVAE2A_INSTALL_DIR}"
   ${ATLAS_ADVAE2A_SOURCE}
   ${ATLAS_ADVAE2A_PATCH}
   CMAKE_CACHE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      -DCMAKE_INSTALL_INCLUDEDIR:PATH=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR:PATH=${CMAKE_INSTALL_LIBDIR}
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
   LOG_CONFIGURE 1
   BUILD_BYPRODUCTS ${ADVAE2A_LIBRARIES} )
ExternalProject_Add_Step( ADVAE2A forcedownload
   COMMAND ${CMAKE_COMMAND} -E echo
   "${ATLAS_ADVAE2A_FORCEDOWNLOAD_MESSAGE}"
   INDEPENDENT TRUE
   DEPENDERS download )
ExternalProject_Add_Step( ADVAE2A purgeBuild
   COMMAND ${CMAKE_COMMAND} -E remove_directory "<BINARY_DIR>"
   COMMAND ${CMAKE_COMMAND} -E make_directory "<BINARY_DIR>"
   COMMENT "Removing previous build results for ADVAE2A."
   INDEPENDENT TRUE
   DEPENDEES download
   DEPENDERS patch )
