# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1TRT )

# Component(s) in the package:
atlas_add_component( TrigT1TRT
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel InDetRawData TRT_ConditionsServicesLib TRT_ReadoutGeometry TrigConfInterfaces TrigConfL1Data TrigT1Interfaces )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
