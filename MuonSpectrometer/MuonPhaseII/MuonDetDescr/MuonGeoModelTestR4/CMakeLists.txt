# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelTestR4 )

if( NOT SIMULATIONBASE )
  find_package( ROOT COMPONENTS Core Tree TreePlayer )
  atlas_add_component( MuonGeoModelTestR4
                       src/components/*.cxx src/*.cxx
                       LINK_LIBRARIES AthenaKernel StoreGateLib GeoModelUtilities MuonTesterTreeLib MuonCablingData
                                      GaudiKernel MuonReadoutGeometryR4 AthenaPoolUtilities CxxUtils
                                      MuonVisualizationHelpersR4 MuonRecToolInterfacesR4)

  # Some functions here make heavy use of Eigen and are thus very slow in debug
  # builds.  Set up to allow forcing them to compile with optimization and
  # inlining, even in debug builds.
  if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    set_source_files_properties(
       ${CMAKE_CURRENT_SOURCE_DIR}/src/MuonChamberToolTest.cxx
       PROPERTIES
       COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
       COMPILE_DEFINITIONS "FLATTEN" )
  endif()

endif()

if (NOT SIMULATIONBASE)
    #### Standard geometry dumping test R3 (MC)
    atlas_add_test( testR3Geometry
                    SCRIPT MuonGeoModelTestR4_testGeometry.sh RUN3
                    PROPERTIES TIMEOUT 1800
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)

    #### Standard geometry dumping test R3-MTech (MC)
    atlas_add_test( testR3Geometry_MTech
                    SCRIPT MuonGeoModelTestR4_testGeometry.sh RUN3 MTECH
                    PROPERTIES TIMEOUT 1800
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)
    #### Standard geometry dumping test R3 (data)
    atlas_add_test( testR3GeometryData
                    SCRIPT MuonGeoModelTestR4_testGeometry.sh RUN3 DATA
                    PROPERTIES TIMEOUT 1800
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)
    #### Standard geometry dumping test R4 (MC)
    atlas_add_test( testR4Geometry
                    SCRIPT MuonGeoModelTestR4_testGeometry.sh RUN4
                    PROPERTIES TIMEOUT 1800
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)

    #### Check consistency of the strip design
    atlas_add_test( checkStripDesign
                    SCRIPT runStripDesignDump                
                    PROPERTIES TIMEOUT 900
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)

    #### Test that the sector envelopes fully enclose the particular chambers
    atlas_add_test( testChamberBuilding
                    SCRIPT python -m MuonGeoModelTestR4.testChamberBuilder 
                    PROPERTIES TIMEOUT 1200
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)
    #### Test that the sector envelopes fully enclose the particular chambers (R4)
    atlas_add_test( testChamberBuildingR4
                    SCRIPT python -m MuonGeoModelTestR4.testChamberBuilder --defaultGeoFile RUN4
                    PROPERTIES TIMEOUT 1200
                    PRIVATE_WORKING_DIRECTORY
                    POST_EXEC_SCRIPT noerror.sh)

    file(GLOB_RECURSE files "util/*.cxx")
    foreach(_exeFile ${files})
        get_filename_component(_theExec ${_exeFile} NAME_WE)
        get_filename_component(_theLoc ${_exeFile} DIRECTORY)
        # we specify a folder for programs we do not want to compile. Useful during r21 transition...
        if(${_theLoc} MATCHES "DoNotBuild")
          continue()
        endif() 
        atlas_add_executable( ${_theExec} ${_exeFile}
                            INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                            LINK_LIBRARIES ${ROOT_LIBRARIES} MuonReadoutGeometryR4 MuonCablingData PathResolver )
    endforeach()
endif()
##### Simulation test R3
atlas_add_test( testR3SensitiveDetectors
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors  --nEvents 2 
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)

##### Simulation test R4
atlas_add_test( testR4SensitiveDetectors
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors  --nEvents 2 --defaultGeoFile RUN4
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT noerror.sh)






# Install files from the package:
atlas_install_scripts( scripts/*.sh )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
