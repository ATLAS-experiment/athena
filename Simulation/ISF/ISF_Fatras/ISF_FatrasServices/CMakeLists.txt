# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FatrasServices )

# Component(s) in the package:
atlas_add_component( ISF_FatrasServices
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES CxxUtils GaudiKernel ISF_InterfacesLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Add unit test
atlas_add_test( ISF_FatrasConfig_test
                SCRIPT python -m ISF_FatrasServices.ISF_FatrasConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )

