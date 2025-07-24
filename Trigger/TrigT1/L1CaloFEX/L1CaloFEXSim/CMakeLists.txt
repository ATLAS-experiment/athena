# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( L1CaloFEXSim )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Gpad Graf )

# Component(s) in the package:
atlas_add_library( L1CaloFEXSimLib
                   L1CaloFEXSim/*.h src/*.cxx
                   PUBLIC_HEADERS L1CaloFEXSim
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} TruthUtils AthContainers AthLinks AthenaBaseComps AthenaKernel CaloEvent CaloIdentifier CxxUtils GaudiKernel Identifier L1CaloFEXToolInterfaces StoreGateLib xAODBase xAODCore xAODTrigL1Calo xAODTruth xAODTrigger L1CaloFEXCondLib
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} AtlasHepMCLib TrigConfData SGTools xAODJet PathResolver)

atlas_add_component( L1CaloFEXSim
                     src/components/*.cxx
		PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} L1CaloFEXSimLib)
                     
# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( share/*.csv )
atlas_install_data( data/bdt_config_v*.json )

atlas_add_test( L1CaloFEXSimCfg_MC
                SCRIPT python -m L1CaloFEXSim.L1CaloFEXSimCfg -i ttbar -e -n 5
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1CaloFEXSimCfg_R2data
                SCRIPT python -m L1CaloFEXSim.L1CaloFEXSimCfg -i data_run2_EB -e -n 5
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1CaloFEXSimCfg_R3data
                SCRIPT python -m L1CaloFEXSim.L1CaloFEXSimCfg -i data -e -n 5
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1CaloFEXSimCfgBDTTau_R3data
                SCRIPT python -m L1CaloFEXSim.L1CaloFEXSimCfg -i data -e -d -n 5
                PRIVATE_WORKING_DIRECTORY
		        ENVIRONMENT "TAU_EFEX_BDT_CONFIG_PATH=${CMAKE_CURRENT_SOURCE_DIR}/data/bdt_config_v16.json"
		        POST_EXEC_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/share/validateBDTTau.py )
