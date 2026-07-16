# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigEDMConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )
atlas_install_scripts( test/*.sh )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( testEDM
   SCRIPT testEDM.py
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( testEDMRun3
   SCRIPT testEDMRun3.py
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( testEDMRun4
   SCRIPT testEDMRun4.py
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( testEDMRun3Reproducibility
   SCRIPT testTriggerEDMRun3Reproducibility.sh
   POST_EXEC_SCRIPT noerror.sh )
