# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_FastCaloSimEvent )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Component(s) in the package:
# Most of the former ISF_FastCaloSimEvent classes are now provided by the
# external FastCaloSim library. Only the Athena-specific G4 step-info classes
# (and the ICaloGeometry / FastCaloSim_CaloCell_ID helper interfaces) remain,
# and none of them depend on the external FastCaloSim library any more (the
# G4 step-info classes use Athena messaging, not FastCaloSim's logger).
atlas_add_library( ISF_FastCaloSimEvent
   ISF_FastCaloSimEvent/*.h src/*.cxx
   PUBLIC_HEADERS ISF_FastCaloSimEvent
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   DEFINITIONS ${CLHEP_DEFINITIONS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES}
   AthContainers AthenaKernel CaloDetDescrLib CaloGeoHelpers CxxUtils TileSimEvent
   PRIVATE_LINK_LIBRARIES GaudiKernel )

# The dictionary for the FCS_StepInfo(Collection) Athena classes:
atlas_add_dictionary( ISF_FastCaloSimEventDict
   ISF_FastCaloSimEvent/ISF_FastCaloSimEventDict.h
   ISF_FastCaloSimEvent/selection.xml
   LINK_LIBRARIES ISF_FastCaloSimEvent )
