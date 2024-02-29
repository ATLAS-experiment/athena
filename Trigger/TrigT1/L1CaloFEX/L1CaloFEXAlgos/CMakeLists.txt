# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1CaloFEXAlgos )

# External dependencies:
find_package( tdaq-common COMPONENTS eformat eformat_write )

find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Gpad Graf )

# Component(s) in the package:
atlas_add_component( L1CaloFEXAlgos
                     src/*.cxx
                     src/components/*.cxx
                     DEFINITIONS OFFLINE_DECODER
                     INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES PathResolver CaloEvent xAODTrigL1Calo ${TDAQ-COMMON_LIBRARIES} ${ROOT_LIBRARIES} L1CaloFEXSimLib LArRecConditions )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
# atlas_install_data( data/* )
