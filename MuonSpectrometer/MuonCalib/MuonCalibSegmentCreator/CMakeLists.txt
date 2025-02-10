atlas_subdir( MuonCalibSegmentCreator )


# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( TBB )
find_package( Eigen )
find_package( CLHEP )

atlas_add_component( MuonCalibSegmentCreator
                    src/*.h src/*.cxx src/components/*.cxx
                    INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                    LINK_LIBRARIES ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${EIGEN_LIBRARIES} ${CLHEP_LIBRARIES} 
                                    GaudiKernel AthenaBaseComps EventInfo MuonSegment TrkSegment MuonTesterTreeLib 
                                    TrkTrack MuonCompetingRIOsOnTrack MuonRecHelperToolsLib MuonRecToolInterfaces MuonRIO_OnTrack MuonPrepRawData MuonCalibITools 
                                    MdtCalibInterfacesLib TrkExInterfaces TrkToolInterfaces)

atlas_install_python_modules( python/*.py )
