atlas_subdir( MuonCalibSegmentCreator )

atlas_add_component( MuonCalibSegmentCreator
                    src/*.h src/*.cxx src/components/*.cxx
                    LINK_LIBRARIES  GaudiKernel AthenaBaseComps EventInfo MuonSegment TrkSegment MuonTesterTreeLib
                                    TrkTrack MuonCompetingRIOsOnTrack MuonRecHelperToolsLib MuonRecToolInterfaces MuonRIO_OnTrack MuonPrepRawData MuonCalibITools 
                                    MdtCalibInterfacesLib TrkExInterfaces TrkToolInterfaces)

atlas_install_python_modules( python/*.py )
