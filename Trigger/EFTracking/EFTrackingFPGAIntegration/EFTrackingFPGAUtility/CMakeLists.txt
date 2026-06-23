# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAUtility)

atlas_add_library(EFTrackingFPGAUtilityLib
                  src/*.cxx
                  PUBLIC_HEADERS EFTrackingFPGAUtility
                  LINK_LIBRARIES GaudiKernel 
                                 AthenaBaseComps 
                                 AthenaMonitoringKernelLib 
                                 xAODInDetMeasurement 
                                 InDetPrepRawData 
                                 InDetIdentifier 
                                 InDetReadoutGeometry
				 FPGATrackSimObjectsLib
			         InDetMeasurementUtilitiesLib)

atlas_add_component(EFTrackingFPGAUtility
                    src/components/*.cxx
                    LINK_LIBRARIES EFTrackingFPGAUtilityLib)

atlas_install_python_modules(python/*.py)
atlas_install_scripts(script/*.sh)

