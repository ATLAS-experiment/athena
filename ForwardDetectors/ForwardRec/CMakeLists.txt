# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ForwardRec )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )   

# Tests in the package:
atlas_add_test( LucidConfig_test
    SCRIPT python -m  ForwardRec.LucidRecConfig 
    POST_EXEC_SCRIPT noerror.sh)
