# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package
atlas_subdir( TrigGepPerf )

# Define macros based on conditions
# For example, you can set these flags based on some conditions or manually
# Here we manually define FLOATING_POINT_SIMULATION
add_definitions(-DFLOATING_POINT_SIMULATION)
# add_definitions(-DBITWISE_SIMULATION)

# External package dependencies:
find_package( ROOT COMPONENTS Core Math Physics )

atlas_add_library( TrigGepPerfLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigGepPerf
                   LINK_LIBRARIES AthContainers AthLinks AthenaKernel xAODTrigger CaloDetDescrLib CaloIdentifier TrigT1Interfaces TrigEFMissingETLib xAODEventInfo xAODJet
                   PRIVATE_LINK_LIBRARIES GaudiKernel PathResolver )

atlas_add_component( TrigGepPerf 
		     src/*.cxx
		     src/components/*.cxx
  		     LINK_LIBRARIES
		     AthenaBaseComps
		     AthContainers
		     CaloDetDescrLib
		     CaloIdentifier
		     GaudiKernel
                     PathResolver
		     TrigT1Interfaces
		     TrigEFMissingETLib
		     xAODEventInfo
		     xAODJet
		     xAODTrigger
)

# Install python modules, joboptions, and share content
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )



