# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsCalibrators )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson )

atlas_add_library( ActsCalibratorsLib
                   src/*.cxx
                   PUBLIC_HEADERS ActsCalibrators
                   LINK_LIBRARIES ActsCore ActsCalibBase ActsGeometryLib TrkEventPrimitives TrkMeasurementBase
                                  TrkSurfaces ActsToolInterfacesLib TrkToolInterfaces GeoPrimitives
                                  TrkRIO_OnTrack InDetMeasurementUtilitiesLib InDetRIO_OnTrack
                    PRIVATE_LINK_LIBRARIES xAODMuonPrepData)

atlas_add_test ( MeasurementCalibratorBaseTest
                 SOURCES test/MeasurementCalibratorBaseTest.cxx
                LINK_LIBRARIES ActsCore ActsEventLib ActsCalibratorsLib ActsGeometryInterfacesLib
                POST_EXEC_SCRIPT nopost.sh )
                                
atlas_add_test ( TrkMeasurementCalibratorTest
                SOURCES test/TrkMeasurementCalibratorTest.cxx
                LINK_LIBRARIES ActsCore ActsCalibratorsLib TestTools AthenaBaseComps
                DEPENDS MeasurementCalibratorBaseTest
                POST_EXEC_SCRIPT nopost.sh )
              