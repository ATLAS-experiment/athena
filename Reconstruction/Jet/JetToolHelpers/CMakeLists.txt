################################################################################
# Package: SSS
################################################################################

# Declare the package name:
atlas_subdir( JetToolHelpers )

# Extra dependencies, based on the environment:
set( extra_deps )
if( NOT GENERATIONBASE )
   list( APPEND extra_libs xAODPFlow )
   if( NOT XAOD_ANALYSIS )
      list( APPEND extra_libs AthenaMonitoringKernelLib )
   endif()
   if( NOT XAOD_STANDALONE )
      list( APPEND extra_libs StoreGateLib )
   endif()
endif()

#if( XAOD_STANDALONE )
#   set( extra_deps Control/xAODRootAccess )
#else()
#   set( extra_deps GaudiKernel )
#endif()


# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Graf Gpad )

# Libraries in the package:
atlas_add_library( JetToolHelpersLib
   JetToolHelpers/*.h Root/*.cxx Root/*.cpp
   PUBLIC_HEADERS JetToolHelpers
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   AsgTools xAODEventInfo xAODJet xAODTracking JetAnalysisInterfacesLib AsgDataHandlesLib AnaAlgorithmLib AthLinks xAODBase xAODCaloEvent ${extra_libs} CxxUtils
   PRIVATE_LINK_LIBRARIES xAODCore)

if( NOT XAOD_STANDALONE )
   atlas_add_component( JetToolHelpers
      src/components/*.cxx
      LINK_LIBRARIES AthenaBaseComps GaudiKernel JetToolHelpersLib StoreGateLib xAODJet AnaAlgorithmLib AthContainers)
endif()

atlas_add_dictionary( JetToolHelpersDict
   JetToolHelpers/JetToolHelpersDict.h
   JetToolHelpers/selection.xml
   LINK_LIBRARIES JetToolHelpersLib )

atlas_install_python_modules( python/*.py )
