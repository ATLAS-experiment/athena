# Declare the package name:
atlas_subdir( PixelToTPIDTool )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist RIO TMVA Tree TreePlayer ROOTDataFrame )
find_package( VDT ) # might need for RDataFrame.  See Control/xAODDataSource/CMakeLists.txt

message(STATUS "XAOD_STANDALONE = ${XAOD_STANDALONE}")

if( NOT XAOD_STANDALONE )
    set(EXTRA_ATHENA_LIBS PixelConditionsData PixelGeoModelLib TrkRIO_OnTrack InDetRIO_OnTrack )
else()
    set(EXTRA_ATHENA_LIBS "")
endif()

# ------------------------------------------------------------------------------
# Shared Utility Library (PixelDEdxUtils)
# ------------------------------------------------------------------------------
atlas_add_library( PixelDEdxUtils
   PixelToTPIDTool/PixelDEdxUtils.h Root/PixelDEdxUtils.cxx
   PUBLIC_HEADERS PixelToTPIDTool
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${VDT_LIBRARIES}
                 AsgTools xAODEventInfo xAODTracking PATCoreLib )

# ------------------------------------------------------------------------------
# AnalysisBase Tool: PixelDEdxEqualizationTool (xAOD only, lives in Root/)
# ------------------------------------------------------------------------------
atlas_add_library( PixelDEdxEqualizationToolLib
   PixelToTPIDTool/PixelDEdxEqualizationTool.h Root/PixelDEdxEqualizationTool.cxx
   PUBLIC_HEADERS PixelToTPIDTool
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${VDT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${VDT_LIBRARIES}
                 AsgTools AsgDataHandlesLib PathResolver
                 xAODEventInfo xAODTracking TrkAnalysisInterfacesLib
                 PixelDEdxUtils PATCoreLib )

# ------------------------------------------------------------------------------
# Athena Tool: PixelToTPIDTool (ESD-based, lives in src/)
# ------------------------------------------------------------------------------
if(NOT XAOD_STANDALONE)
   atlas_add_library( PixelToTPIDToolLib
      PixelToTPIDTool/PixelToTPIDTool.h src/PixelToTPIDTool.cxx
      #PUBLIC_HEADERS PixelToTPIDTool
      #INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      HEADERS PixelToTPIDTool/PixelToTPIDTool.h  # now private
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES}
                    AsgTools xAODEventInfo xAODTracking  TrkToolInterfaces
                    ${EXTRA_ATHENA_LIBS}
                    PixelDEdxUtils PATCoreLib )
endif()

# Athena Component
if( NOT XAOD_STANDALONE )
   atlas_add_component( PixelToTPIDTool
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES PixelToTPIDToolLib )
endif()


# ------------------------------------------------------------------------------
# Dictionary Generation
# ------------------------------------------------------------------------------
if(NOT XAOD_STANDALONE)
   atlas_add_dictionary( PixelToTPIDToolDict
      PixelToTPIDTool/PixelToTPIDToolDict.h
      PixelToTPIDTool/selection.xml
      LINK_LIBRARIES PixelToTPIDToolLib PixelDEdxEqualizationToolLib PixelDEdxUtils )
endif()

# ------------------------------------------------------------------------------
# Executables (only in AnalysisBase)
# ------------------------------------------------------------------------------
if( XAOD_STANDALONE )
   atlas_add_executable( PixelDEdxEqualizationToolTester
      util/PixelDEdxEqualizationToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon
      xAODTracking xAODCore PixelDEdxEqualizationToolLib PATCoreLib )
endif()

# ------------------------------------------------------------------------------
# Python
# ------------------------------------------------------------------------------
#atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
