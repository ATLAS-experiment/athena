# Declare the package name:
atlas_subdir( PixelToTPIDTool )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO TMVA Tree TreePlayer ROOTDataFrame )
find_package( VDT ) # might need for RDataFrame.  See Control/xAODDataSource/CMakeLists.txt
#find_package( CLHEP )

message(STATUS "XAOD_STANDALONE = ${XAOD_STANDALONE}")

if( NOT XAOD_STANDALONE )
    set(EXTRA_ATHENA_LIBS PixelConditionsData PixelGeoModelLib TrkRIO_OnTrack InDetRIO_OnTrack)
else()
    set(EXTRA_ATHENA_LIBS "")
endif()

# Libraries in the package:
atlas_add_library( PixelToTPIDToolLib
   PixelToTPIDTool/*.h Root/*.cxx
   PUBLIC_HEADERS PixelToTPIDTool
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS} # ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${VDT_LIBRARIES} # ${CLHEP_LIBRARIES}
                 AsgTools xAODEventInfo PATCoreLib AsgDataHandlesLib PATInterfaces
                 xAODTracking PathResolver TrkAnalysisInterfacesLib 
                 ${EXTRA_ATHENA_LIBS} )

# Athena Component Library
if( NOT XAOD_STANDALONE )
   atlas_add_component( PixelToTPIDTool
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES PixelToTPIDToolLib )
endif()

# Dictionary Generation
atlas_add_dictionary( PixelToTPIDToolDict
   PixelToTPIDTool/PixelToTPIDToolDict.h
   PixelToTPIDTool/selection.xml
   LINK_LIBRARIES PixelToTPIDToolLib )


# Executable(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_executable( PixelToTPIDToolTester
      util/PixelToTPIDToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon
      xAODTracking xAODCore PixelToTPIDToolLib PATCoreLib )
endif()



# Test(s) in the package:

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
