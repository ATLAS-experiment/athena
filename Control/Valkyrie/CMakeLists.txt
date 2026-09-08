# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( Valkyrie )

#Only install C++/Python modules in full Athena
if( NOT GENERATIONBASE AND NOT SIMULATIONBASE AND NOT XAOD_ANALYSIS )
# External dependencies:
find_package( Boost COMPONENTS regex )
find_package( valgrind )

# Component(s) in the package:
atlas_add_component( Valkyrie
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${VALGRIND_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${VALGRIND_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
endif()
atlas_install_data( share/valgrind-*.supp )
atlas_install_scripts( share/valgrind-atlas-opts.sh )
