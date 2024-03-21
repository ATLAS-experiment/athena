# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TBDetDescrAlg )

# Component(s) in the package:
atlas_add_component( TBDetDescrAlg
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_LINK_LIBRARIES AthenaBaseComps DetDescrCnvSvcLib GaudiKernel TBDetDescr )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

