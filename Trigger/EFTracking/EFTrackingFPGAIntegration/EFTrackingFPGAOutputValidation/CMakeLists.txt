# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAOutputValidation)

atlas_add_component(EFTrackingFPGAOutputValidation
                    src/*.cxx
                    src/components/*.cxx
                    LINK_LIBRARIES GaudiKernel
                                   AthenaBaseComps 
                                   AthenaMonitoringKernelLib 
                                   xAODInDetMeasurement 
                                   InDetPrepRawData 
                                   InDetIdentifier 
                                   InDetReadoutGeometry
				   InDetMeasurementUtilitiesLib )

atlas_install_python_modules(python/*.py)

