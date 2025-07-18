# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODAuxillaryMeasurementAthenaPool )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_poolcnv_library( xAODAuxillaryMeasAthenaPoolCnv
   FILES xAODAuxillaryMeasurement/AuxillaryMeasurementContainer.h 
         xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer1D.h 
         xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer2D.h 
         xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer3D.h 
                  
   TYPES_WITH_NAMESPACE xAOD::AuxillaryMeasurementContainer 
                        xAOD::AuxillaryMeasurementAuxContainer1D
                        xAOD::AuxillaryMeasurementAuxContainer2D
                        xAOD::AuxillaryMeasurementAuxContainer3D
                       
   CNV_PFX xAOD
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel
                  AthenaPoolCnvSvcLib AthenaPoolUtilities xAODAuxillaryMeasurement)

