# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODAuxiliaryMeasurement )

# External dependencies.
find_package( ROOT COMPONENTS Core GenVector )
find_package( xAODUtilities )

# Extra dependencies, based on the build environment.
if( NOT XAOD_ANALYSIS AND NOT GENERATIONBASE )
    find_package(Acts COMPONENTS Core)
    # Component(s) in the package.
    atlas_add_library( xAODAuxiliaryMeasurement
       xAODAuxiliaryMeasurement/*.h xAODAuxiliaryMeasurement/versions/*.h Root/*.cxx
       PUBLIC_HEADERS xAODAuxiliaryMeasurement
       INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
       LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthLinks xAODBase xAODCore 
                     xAODTracking xAODMeasurementBase ActsCore ActsCalibBase ActsGeoUtils)

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODAuxiliaryMeasurement/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::AuxiliaryMeasurementContainer_v1")

atlas_add_dictionary( xAODAuxiliaryMeasurementDict
                      xAODAuxiliaryMeasurement/xAODAuxiliaryMeasurementDict.h
                      ${_selectionFile}
                      LINK_LIBRARIES xAODCore xAODAuxiliaryMeasurement
                      EXTRA_FILES Root/dict/*.cxx )
endif()

