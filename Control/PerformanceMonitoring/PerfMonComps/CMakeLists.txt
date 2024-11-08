# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonComps )

# External dependencies:
find_package( TBB )
find_package( nlohmann_json )
find_package( psutil )
find_package( matplotlib )
find_package( numpy )
find_package( pandas )
find_package( sqlalchemy )
find_package( zipp )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( PerfMonComps
   src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES ${TBB_LIBRARIES} AthenaBaseComps CxxUtils GaudiKernel PerfMonKernel nlohmann_json::nlohmann_json )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
# Install files from the package:
atlas_install_scripts( bin/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
