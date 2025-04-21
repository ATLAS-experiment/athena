# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthExFortranAlgorithm )

# Component(s) in the package:
atlas_add_component( AthExFortranAlgorithm
                     src/*.cxx
                     src/*.F
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel )

# Install files from the package:
atlas_install_runtime( share/FortranAlgorithmInput.data )

# Tests in the package:
atlas_add_test( FortranAlgorithm
   PRE_EXEC_SCRIPT "get_files -data FortranAlgorithmInput.data"
   SCRIPT share/FortranAlgorithmConfig.py
   LOG_SELECT_PATTERN "^F: " )
