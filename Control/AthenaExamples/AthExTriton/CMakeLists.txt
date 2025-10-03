# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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

# disabled the unit tests for now  as they require a Triton server available.

# Test the packages
# atlas_add_test( AthExTritonTest
#    SCRIPT python -m AthExTriton.AthExTriton_test
#    POST_EXEC_SCRIPT noerror.sh )

# atlas_add_test( AthExTritonTest_pkl
#    SCRIPT athena --evtMax 2 --CA test_AthExTritonCfg.pkl
#    DEPENDS AthExTritonTest
#    POST_EXEC_SCRIPT noerror.sh )
