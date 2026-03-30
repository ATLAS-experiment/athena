# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AtlasGeoModel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if( NOT SIMULATIONBASE AND NOT GENERATIONBASE )
  atlas_add_test( AtlasGeoModelConfig
                  SCRIPT python -m AtlasGeoModel.GeoModelConfig
                  POST_EXEC_SCRIPT noerror.sh )
endif()
