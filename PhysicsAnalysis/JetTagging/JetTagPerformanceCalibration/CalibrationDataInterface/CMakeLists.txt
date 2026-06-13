# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CalibrationDataInterface )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO MathMore Minuit Minuit2 Matrix Physics HistPainter Rint)
find_package( nlohmann_json )
find_package( Boost ) # header-only so no libraries needed

# Component(s) in the package:
atlas_add_root_dictionary( CalibrationDataInterfaceLib
                           CalibrationDataInterfaceLibDictSource
                           ROOT_HEADERS
                           CalibrationDataInterface/CalibrationDataContainer.h
                           CalibrationDataInterface/CalibrationDataVariables.h
                           CalibrationDataInterface/CalibrationDataInterfaceBase.h
                           CalibrationDataInterface/CalibrationDataInterfaceROOT.h
                           CalibrationDataInterface/CalibrationDataEigenVariations.h
                           CalibrationDataInterface/CalibrationDataBackend.h
                           CalibrationDataInterface/CalibrationDataUtilities.h
                           CalibrationDataInterface/CalibrationDataInternals.h
                           CalibrationDataInterface/CDIReader.h
                           Root/LinkDef.h
                           EXTERNAL_PACKAGES ROOT Boost )

atlas_add_library( CalibrationDataInterfaceLib
                   Root/*.cxx
                   ${CalibrationDataInterfaceLibDictSource}
                   PUBLIC_HEADERS CalibrationDataInterface
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} CxxUtils
                   PRIVATE_LINK_LIBRARIES nlohmann_json::nlohmann_json )

atlas_add_executable( CDIReaderTester
                      src/CDIReaderTester.cxx
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} CxxUtils nlohmann_json::nlohmann_json CalibrationDataInterfaceLib)

# Install files from the package:
atlas_install_joboptions( share/*.py )
atlas_install_python_modules( python/*.py )
