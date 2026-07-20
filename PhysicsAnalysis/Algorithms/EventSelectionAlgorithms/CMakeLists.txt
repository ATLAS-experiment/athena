# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina

atlas_subdir( EventSelectionAlgorithms )

find_package( ROOT COMPONENTS Core )

atlas_add_library( EventSelectionAlgorithmsLib
   EventSelectionAlgorithms/*.h EventSelectionAlgorithms/*.icc Root/*.cxx
   PUBLIC_HEADERS EventSelectionAlgorithms
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AnaAlgorithmLib FourMomUtils SystematicsHandlesLib SelectionHelpersLib xAODJet xAODMuon xAODTau xAODEgamma xAODMissingET
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} PATInterfaces xAODCore xAODMetaData  )

atlas_add_dictionary( EventSelectionAlgorithmsDict
   EventSelectionAlgorithms/EventSelectionAlgorithmsDict.h
   EventSelectionAlgorithms/selection.xml
   LINK_LIBRARIES EventSelectionAlgorithmsLib )

atlas_add_component( EventSelectionAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES EventSelectionAlgorithmsLib )

atlas_install_python_modules( python/*.py )

atlas_install_data( data/* )

atlas_add_test( EventSelectionConfigHelpers
   SCRIPT python/EventSelectionConfigHelpers_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 30 )

atlas_add_test( EventSelectionConfigDispatch
   SCRIPT python/EventSelectionConfigDispatch_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 300 )

atlas_add_test( EventSelectionConfigNObject
   SCRIPT python/EventSelectionConfigNObject_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 300 )

atlas_add_test( EventSelectionConfigBTag
   SCRIPT python/EventSelectionConfigBTag_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigGhost
   SCRIPT python/EventSelectionConfigGhost_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigMass
   SCRIPT python/EventSelectionConfigMass_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigSumLeptons
   SCRIPT python/EventSelectionConfigSumLeptons_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigMetMwt
   SCRIPT python/EventSelectionConfigMetMwt_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigDilepton
   SCRIPT python/EventSelectionConfigDilepton_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigChaining
   SCRIPT python/EventSelectionConfigChaining_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

atlas_add_test( EventSelectionConfigMerger
   SCRIPT python/EventSelectionConfigMerger_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 60 )

 atlas_add_test( EventSelectionConfigExpr
   SCRIPT python/EventSelectionConfigExpr_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )

 atlas_add_test( EventSelectionConfigEventVar
   SCRIPT python/EventSelectionConfigEventVar_unitTest.py
   POST_EXEC_SCRIPT nopost.sh
   PROPERTIES TIMEOUT 120 )
