# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAUtility)

atlas_add_library(EFTrackingFPGAUtilityLib
                  src/* 
                  PUBLIC_HEADERS EFTrackingFPGAUtility
                  LINK_LIBRARIES GaudiKernel 
                                 AthenaBaseComps 
                                 AthenaMonitoringKernelLib 
                                 xAODInDetMeasurement 
                                 InDetPrepRawData 
                                 InDetIdentifier 
                                 InDetReadoutGeometry
			         InDetMeasurementUtilitiesLib)

atlas_add_component(EFTrackingFPGAUtility
                    src/components/*
                    PUBLIC_HEADERS EFTrackingFPGAUtility
                    LINK_LIBRARIES EFTrackingFPGAUtilityLib)

atlas_install_python_modules(python/*.py)
atlas_install_scripts(script/*.sh)

