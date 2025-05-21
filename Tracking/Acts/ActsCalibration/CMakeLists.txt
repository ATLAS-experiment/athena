# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsCalibration )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson )

atlas_add_library( ActsCalibrationLib
                   src/*.cxx
                   PUBLIC_HEADERS ActsCalibration
                   LINK_LIBRARIES ActsCore ActsGeometryLib TrkEventPrimitives TrkMeasurementBase
                                  TrkSurfaces ActsToolInterfacesLib TrkToolInterfaces GeoPrimitives
                                  TrkRIO_OnTrack xAODMuonPrepData)

atlas_add_test ( MeasurementCalibratorBaseTest
                 SOURCES test/MeasurementCalibratorBaseTest.cxx
                LINK_LIBRARIES ActsCore ActsEventLib ActsCalibrationLib ActsGeometryInterfacesLib
                POST_EXEC_SCRIPT nopost.sh )
                                
atlas_add_test ( TrkMeasurementCalibratorTest
                SOURCES test/TrkMeasurementCalibratorTest.cxx
                LINK_LIBRARIES ActsCore ActsCalibrationLib TestTools AthenaBaseComps
                DEPENDS MeasurementCalibratorBaseTest
                POST_EXEC_SCRIPT nopost.sh )
              