# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Recipe for building ARTEMIS2A as part of the package.
#

# CMake include(s).
include( ExternalProject )

# Declare where to get ARTEMIS2A from
set( ATLAS_ARTEMIS2A_SOURCE
   "URL;https://atlas-software-dist-eos.web.cern.ch/externals/AnomDetVAE2A/l-1-topo-2-a-artemis-v1.0.0.tar.gz;https://gitlab.cern.ch/atlas-l1calo/l1topo/specialAlgorithms/anomaly-detection-vae/l-1-topo-2-a-artemis/-/archive/v1.0.0/l-1-topo-2-a-artemis-v1.0.0.tar.gz;URL_MD5;e2570441d2478ff68bfa1109f3e139a4"
   CACHE STRING "Source for the ARTEMIS2A project" )
set( ATLAS_ARTEMIS2A_PATCH ""
   CACHE STRING "Patch command for ARTEMIS2A" )
set( ATLAS_ARTEMIS2A_FORCEDOWNLOAD_MESSAGE
   "Forcing the re-download of ARTEMIS2A"
   CACHE STRING "Download message to update whenever patching changes" )
mark_as_advanced( ATLAS_ARTEMIS2A_SOURCE ATLAS_ARTEMIS2A_PATCH
   ATLAS_ARTEMIS2A_FORCEDOWNLOAD_MESSAGE )

# Files / directories produced by the following build.
set( ARTEMIS2A_INSTALL_DIR
   "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/artemis2a-install" )
set( ARTEMIS2A_INCLUDE_DIRS
   "${ARTEMIS2A_INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}" )
set( ARTEMIS2A_LIBRARIES
   "${ARTEMIS2A_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}artemis2a${CMAKE_STATIC_LIBRARY_SUFFIX}" )

# Build ARTEMIS2A into a static library, that would only be used privately
# by this package.
ExternalProject_Add( ARTEMIS2A
   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   PREFIX "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
   INSTALL_DIR "${ARTEMIS2A_INSTALL_DIR}"
   ${ATLAS_ARTEMIS2A_SOURCE}
   ${ATLAS_ARTEMIS2A_PATCH}
   CMAKE_CACHE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      -DCMAKE_INSTALL_INCLUDEDIR:PATH=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR:PATH=${CMAKE_INSTALL_LIBDIR}
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
   LOG_CONFIGURE 1
   BUILD_BYPRODUCTS ${ARTEMIS2A_LIBRARIES} )
ExternalProject_Add_Step( ARTEMIS2A forcedownload
   COMMAND ${CMAKE_COMMAND} -E echo
   "${ATLAS_ARTEMIS2A_FORCEDOWNLOAD_MESSAGE}"
   INDEPENDENT TRUE
   DEPENDERS download )
ExternalProject_Add_Step( ARTEMIS2A purgeBuild
   COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/artemis2a"
   COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/artemis2a"
   COMMENT "Removing previous build results for ARTEMIS2A."
   INDEPENDENT TRUE
   DEPENDEES download
   DEPENDERS patch )
