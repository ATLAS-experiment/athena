# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_Calibration )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core MathCore )

atlas_add_library( HGTD_CalibrationLib
                  PUBLIC_HEADERS HGTD_Calibration
                  INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                  LINK_LIBRARIES ${CLHEP_LIBRARIES} HGTD_RawData AthenaKernel
                  AthenaBaseComps SiDigitization GaudiKernel InDetSimEvent)

atlas_add_component( HGTD_Calibration
                  src/*.cxx
                  src/components/*.cxx
                  LINK_LIBRARIES HGTD_CalibrationLib)

atlas_install_python_modules( python/*.py )
