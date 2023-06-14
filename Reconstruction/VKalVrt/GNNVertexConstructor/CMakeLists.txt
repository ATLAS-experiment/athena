# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GNNVertexConstructor )

atlas_add_component( GNNVertexConstructor 
                     src/components/*.cxx
                     LINK_LIBRARIES GNNVertexConstructorLib AthenaBaseComps )


atlas_add_library(   GNNVertexConstructorLib
                     src/*.cxx
                     PUBLIC_HEADERS GNNVertexConstructor 
                     LINK_LIBRARIES AthenaBaseComps StoreGateLib xAODTracking )
                     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
