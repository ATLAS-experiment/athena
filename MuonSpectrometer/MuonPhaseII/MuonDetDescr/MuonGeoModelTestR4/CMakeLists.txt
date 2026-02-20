# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelTestR4 )
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

#### Standard geometry dumping test R3 (MC)
atlas_add_test( testR3Geometry
                SCRIPT python -m MuonGeoModelTestR4.testGeoModel --chambers STL1A1 BML1A3 T1E1A01 MML1A1
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

#### Standard geometry dumping test R3-MTech (MC)
atlas_add_test( testR3Geometry_MTech
                SCRIPT python -m MuonGeoModelTestR4.testGeoModel --chambers BML1A3 T1E1A01 MML1A1 STS1A1 --geoModelFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-R3S-2021-03-02-00_MTech.db
                PROPERTIES TIMEOUT 600
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
#### Standard geometry dumping test R3 (data)
atlas_add_test( testR3GeometryData
                SCRIPT python -m MuonGeoModelTestR4.testGeoModel  --chambers BML1A3 T1E1A01 MML1A1 STL1A1 --inputFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)



#### Standard geometry dumping test R4 (MC)
atlas_add_test( testR4Geometry
                SCRIPT python -m MuonGeoModelTestR4.testGeoModel --chambers BML1A3 T1E1A01 MML1A1 STL1A1 --defaultGeoFile RUN4
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

#### Check consistency of the strip design
atlas_add_test( checkStripDesign
                SCRIPT runStripDesignDump                
                PROPERTIES TIMEOUT 900
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

##### Simulation test R3
atlas_add_test( testR3SensitiveDetectors
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors  --nEvents 2 
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)

##### Simulation test R4
atlas_add_test( testR4SensitiveDetectors
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors  --nEvents 2 --defaultGeoFile RUN4
                PROPERTIES TIMEOUT 1800
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
#### Test that the sector envelopes fully enclose the particular chambers
atlas_add_test( testChamberBuilding
                SCRIPT python -m MuonGeoModelTestR4.testChamberBuilder 
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)
#### Test that the sector envelopes fully enclose the particular chambers (R4)
atlas_add_test( testChamberBuildingR4
                SCRIPT python -m MuonGeoModelTestR4.testChamberBuilder --defaultGeoFile RUN4
                PROPERTIES TIMEOUT 1200
                PRIVATE_WORKING_DIRECTORY
                POST_EXEC_SCRIPT nopost.sh)


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

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
