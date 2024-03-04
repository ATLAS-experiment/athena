# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DCSCalculator2 )

# External dependencies:
find_package( sqlalchemy )
find_package( ipython )

# Install files from the package:
atlas_install_python_modules( python/*.py python/subdetectors POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/*.py )

# Test: does DCS Calculator work?
atlas_add_test( DCSCRun
   SCRIPT dcsc.py -r456685 -d 'sqlite://$<SEMICOLON>schema=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/AthDataQuality/dcscalc_input_456685.db$<SEMICOLON>dbname=CONDBR2'
   POST_EXEC_SCRIPT noerror.sh
   PROPERTIES TIMEOUT 600 
   LOG_SELECT_PATTERN "DCS Calculator failed"
   ENVIRONMENT "PBEAST_SERVER_HTTPS_PROXY=atlasgw.cern.ch:3128"
   )
