# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSecVertexValidation )

# Extra dependencies, based on the build environment:
set( extra_libs )
if( XAOD_STANDALONE )
   set( extra_libs xAODRootAccess EventLoop )
else()
   set( extra_libs AthAnalysisBaseCompsLib )
endif()

# External dependencies:
find_package( ROOT COMPONENTS Core Tree Hist RIO )

if( NOT XAOD_STANDALONE )
    atlas_add_component( InDetSecVertexValidation
                         src/*.cxx src/*.h
                         src/components/*.cxx
                         LINK_LIBRARIES ${extra_libs} GaudiKernel xAODTracking AthenaBaseComps AsgTools AthenaMonitoringLib TrkValHistUtils )
endif()

atlas_install_python_modules(python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
