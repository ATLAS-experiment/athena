atlas_subdir( MuonCalibSegmentCreator )

# Component(s) in the package:
# atlas_add_library( MuonCalibSegmentCreatorLib
#                    src/*.cxx
#                    PUBLIC_HEADERS MuonCalibPatRec
#                    LINK_LIBRARIES AthenaBaseComps GaudiKernel MuonCalibEvent MuonSegment TrkSegment MdtCalibSvcLib MuonIdHelpersLib
#                    PRIVATE_LINK_LIBRARIES Identifier MuonCalibEventBase MuonCalibITools  MuonReadoutGeometry MuonRecTools MuonPattern MuonRIO_OnTrack MuonRecToolInterfaces TrkCompetingRIOsOnTrack TrkEventPrimitives TrkParameters )

# atlas_add_component( MuonCalibSegmentCreator
#                      src/components/*.cxx
#                      LINK_LIBRARIES AthenaBaseComps GaudiKernel MuonCalibEvent MuonSegment TrkSegment Identifier MdtCalibSvcLib MuonCalibEventBase MuonCalibITools MuonReadoutGeometry MuonIdHelpersLib MuonPattern MuonRIO_OnTrack MuonRecToolInterfaces TrkCompetingRIOsOnTrack TrkEventPrimitives TrkParameters MuonCalibPatRecLib MuonTesterTree )

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
                                    TrkTrack MuonCompetingRIOsOnTrack MuonRecHelperToolsLib MuonRecToolInterfaces MuonRIO_OnTrack MuonPrepRawData MuonCalibITools MdtCalibSvcLib TrkExInterfaces TrkToolInterfaces)

atlas_install_python_modules( python/*.py )

#atlas_install_joboptions( share/*.py )
#atlas_install_runtime( share/* )


