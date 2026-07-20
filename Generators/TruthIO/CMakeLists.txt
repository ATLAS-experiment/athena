# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TruthIO )


# Component(s) in the package:
atlas_add_library( TruthIOLib
                     src/*.cxx
                     PUBLIC_HEADERS TruthIO
                     LINK_LIBRARIES AtlasHepMCLib AthenaBaseComps EventInfo GaudiKernel GeneratorModulesLib StoreGateLib xAODEventInfo GeneratorObjects )

atlas_add_component( TruthIO
                     src/components/*.cxx
                     LINK_LIBRARIES TruthIOLib )

# Install files from the package:
atlas_install_joboptions( share/common/*.py )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/POOLtoHEPMC.py )
