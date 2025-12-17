# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeneratorConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

if (NOT XAOD_STANDALONE)
    atlas_add_component( GeneratorConfig
                         src/*.cxx
                         src/components/*.cxx
                         NO_PUBLIC_HEADERS
                         LINK_LIBRARIES AthenaBaseComps GaudiKernel )
endif()

atlas_install_data( data/* )
