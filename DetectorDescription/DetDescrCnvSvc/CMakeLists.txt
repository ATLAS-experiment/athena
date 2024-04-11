# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DetDescrCnvSvc )

# Component(s) in the package:
atlas_add_library( DetDescrCnvSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS DetDescrCnvSvc
                   LINK_LIBRARIES GaudiKernel
                   PRIVATE_LINK_LIBRARIES StoreGateLib AthenaBaseComps)

atlas_add_component( DetDescrCnvSvc
                     src/components/*.cxx
                     LINK_LIBRARIES DetDescrCnvSvcLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

