# Declare the package name:
atlas_subdir( PixelToTPIDDualTool )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO TMVA Tree TreePlayer ROOTDataFrame )
find_package( VDT ) # might need for RDataFrame.  See Control/xAODDataSource/CMakeLists.txt
#find_package( CLHEP )

message(STATUS "XAOD_STANDALONE = ${XAOD_STANDALONE}")

if( NOT XAOD_STANDALONE )
    set(EXTRA_ATHENA_LIBS PixelConditionsData PixelGeoModelLib)
else()
    set(EXTRA_ATHENA_LIBS "")
endif()

# Libraries in the package:
atlas_add_library( PixelToTPIDDualToolLib
   PixelToTPIDDualTool/*.h Root/*.cxx
   PUBLIC_HEADERS PixelToTPIDDualTool
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS} # ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${VDT_LIBRARIES} # ${CLHEP_LIBRARIES}
                 AsgTools xAODEventInfo PATCoreLib AsgDataHandlesLib PATInterfaces
                 xAODTracking PathResolver TrkAnalysisInterfacesLib 
                 ${EXTRA_ATHENA_LIBS} )

# Athena-specific dependencies (Not in XAOD_STANDALONE)
if( NOT XAOD_STANDALONE )
    message(STATUS "Not in STANDALONE, so adding more libraries ")
    target_link_libraries(PixelToTPIDDualToolLib
        PRIVATE TrkTrack TrkRIO_OnTrack InDetRIO_OnTrack
                TrkSurfaces TrkEventPrimitives TrkMeasurementBase  
                InDetIdentifier StoreGateLib GaudiKernel TrkToolInterfaces )
endif()

# Athena Component Library
if( NOT XAOD_STANDALONE )
   atlas_add_component( PixelToTPIDDualTool
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES AthenaBaseComps GaudiKernel xAODCore PathResolver xAODTracking
                     PixelToTPIDDualToolLib PATCoreLib Identifier InDetIdentifier 
                     InDetRIO_OnTrack TrkToolInterfaces TrkAnalysisInterfacesLib )
endif()

# Dictionary Generation
atlas_add_dictionary( PixelToTPIDDualToolDict
   PixelToTPIDDualTool/PixelToTPIDDualToolDict.h
   PixelToTPIDDualTool/selection.xml
   LINK_LIBRARIES PixelToTPIDDualToolLib )


# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( PixelToTPIDDualToolTester
      util/PixelToTPIDDualToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon
      xAODTracking xAODCore PixelToTPIDDualToolLib PATCoreLib )
endif()



# Test(s) in the package:

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
