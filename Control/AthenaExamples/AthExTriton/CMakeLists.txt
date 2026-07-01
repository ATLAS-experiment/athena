# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package's name.
atlas_subdir( AthExTriton )

# Component(s) in the package.
atlas_add_component( AthExTriton
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps GaudiKernel PathResolver AthOnnxInterfaces
)

# Install files from the package:
atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Test the packages
atlas_add_test( AthExTritonTest
   SCRIPT python -m AthExTriton.AthExTriton_test
   LOG_IGNORE_PATTERN "ERROR Locating dev file"  # OK for this example unit test
   PROPERTIES SKIP_RETURN_CODE 2
   POST_EXEC_SCRIPT noerror.sh )

# This is still disabled because it would fail if the above test is skipped
# atlas_add_test( AthExTritonTest_pkl
#    SCRIPT athena --evtMax 2 test_AthExTritonCfg.pkl
#    DEPENDS AthExTritonTest
#    PROPERTIES SKIP_RETURN_CODE 2
#    POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( AthExTritonAsyncTest
   SCRIPT python -m AthExTriton.AthExTritonAsync_test
   LOG_IGNORE_PATTERN "ERROR Locating dev file"  # OK for this example unit test
   PROPERTIES SKIP_RETURN_CODE 2
   POST_EXEC_SCRIPT noerror.sh )
