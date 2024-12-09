# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigCaloHypo )

find_package( tdaq-common COMPONENTS hltinterface )
find_package( Boost )

# Component(s) in the package:
atlas_add_library( TrigCaloHypoLib
                   TrigCaloHypo/*.h
                   INTERFACE
                   PUBLIC_HEADERS TrigCaloHypo
                   LINK_LIBRARIES xAODTrigCalo CaloEvent DecisionHandlingLib GaudiKernel TrigCompositeUtilsLib TrigSteeringEvent )

atlas_add_component( TrigCaloHypo
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${TDAQ-COMMON_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${TDAQ-COMMON_LIBRARIES} AthenaBaseComps AthenaKernel AthenaMonitoringKernelLib AthViews xAODTrigCalo CaloEvent CaloInterfaceLib DecisionHandlingLib GaudiKernel LArRecConditions LArRecEvent StoreGateLib TrigCaloHypoLib TrigCompositeUtilsLib TrigSteeringEvent )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 )

# Install IS schema files:
atlas_install_generic( schema/Larg.LArNoiseBurstCandidates.is.schema.xml
   DESTINATION share/schema
)
