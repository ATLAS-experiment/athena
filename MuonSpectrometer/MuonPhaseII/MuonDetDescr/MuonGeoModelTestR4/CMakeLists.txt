# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonGeoModelTestR4 )

atlas_add_component( MuonGeoModelTestR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib GeoModelUtilities MuonTesterTreeLib
                                    GaudiKernel MuonReadoutGeometryR4 MuonGeoModelR4Lib AthenaPoolUtilities
                                    xAODTruth xAODMuonSimHit MuonStationGeoHelpersLib CxxUtils)


atlas_add_test( testR4Geometry
                SCRIPT python -m MuonGeoModelTestR4.testGeoModel --chambers "BML1A3 T1E1A01" --noMM --noSTGC 
                PROPERTIES TIMEOUT 600
                POST_EXEC_SCRIPT nopost.sh)


atlas_add_test( testR4SensitiveDetectors
                SCRIPT python -m MuonGeoModelTestR4.testSensitiveDetectors  --nEvents 10 
                PROPERTIES TIMEOUT 600
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
