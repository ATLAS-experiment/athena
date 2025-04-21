# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir(EFTrackingFPGAOutputValidation)

atlas_add_component(EFTrackingFPGAOutputValidation
                    src/*.cxx
                    src/components/*.cxx
                    PUBLIC_HEADERS EFTrackingFPGAOutputValidation 
                    LINK_LIBRARIES GaudiKernel 
                                   AthenaBaseComps 
                                   AthenaMonitoringKernelLib 
                                   xAODInDetMeasurement 
                                   InDetPrepRawData 
                                   InDetIdentifier 
                                   InDetReadoutGeometry)

atlas_install_python_modules(python/*.py)

