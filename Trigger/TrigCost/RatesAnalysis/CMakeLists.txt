# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RatesAnalysis )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Package library:
atlas_add_library( RatesAnalysisLib
                   src/Rates*.cxx
		   src/L1Topo*cxx
                   PUBLIC_HEADERS RatesAnalysis
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} GaudiKernel AthAnalysisBaseCompsLib AthenaBaseComps TrigDecisionToolLib EnhancedBiasWeighterLib
                   PRIVATE_LINK_LIBRARIES TrigConfData TrigConfL1Data xAODEgamma xAODEventInfo )

# Suppress spurious warnings seen in the LTO build originating 
# in the boost parser code.
set_target_properties( RatesAnalysisLib
                       PROPERTIES LINK_FLAGS " -Wno-maybe-uninitialized "  )

# Component(s) in the package:
atlas_add_component( RatesAnalysis
                     src/FullMenu.cxx
                     src/components/RatesAnalysis_entries.cxx
                     LINK_LIBRARIES RatesAnalysisLib )

atlas_add_test( RatesAnalysis_test
                SOURCES test/RatesAnalysis_test.cxx
                LINK_LIBRARIES RatesAnalysisLib CxxUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data(share/triggers.json)
atlas_install_scripts(share/L1TopoRatesCalculator_submatrix_plotter.py share/runL1TopoRates.py python/RatesAnalysisFullMenu.py python/RatesEmulationExample.py python/RatesAnalysisPostProcessing.py python/RatesAnalysisOnlineProcessing.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
