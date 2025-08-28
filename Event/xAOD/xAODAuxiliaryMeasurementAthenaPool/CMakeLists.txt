# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODAuxiliaryMeasurementAthenaPool )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODAuxiliaryMeasAthenaPoolCnv
   FILES xAODAuxiliaryMeasurement/AuxiliaryMeasurementContainer.h 
         xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer1D.h 
         xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer2D.h 
         xAODAuxiliaryMeasurement/AuxiliaryMeasurementAuxContainer3D.h 
                  
   TYPES_WITH_NAMESPACE xAOD::AuxiliaryMeasurementContainer 
                        xAOD::AuxiliaryMeasurementAuxContainer1D
                        xAOD::AuxiliaryMeasurementAuxContainer2D
                        xAOD::AuxiliaryMeasurementAuxContainer3D
                       
   CNV_PFX xAOD
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel
                  AthenaPoolCnvSvcLib AthenaPoolUtilities xAODAuxiliaryMeasurement)

