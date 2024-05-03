# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_ActsTools )

find_package( Acts CONFIG COMPONENTS Core PluginJson Fatras )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( ISF_ActsTools
                     src/*.cxx
                     src/components/*.cxx
		             INCLUDE_DIRS
                     ${CLHEP_INCLUDE_DIRS} ${Acts_INCLUDE_DIR}
                     LINK_LIBRARIES ActsCore ActsFatras AthenaBaseComps CxxUtils
                     TrkTrack ISF_Event AthenaKernel AtlasDetDescr GeoPrimitives
                     GaudiKernel GeneratorObjects ISF_Event StoreGateLib
                     GeoModelUtilities ActsInteropLib ActsGeometryLib TruthUtils
                     ISF_InterfacesLib ActsPluginJson ISF_FatrasInterfaces )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
