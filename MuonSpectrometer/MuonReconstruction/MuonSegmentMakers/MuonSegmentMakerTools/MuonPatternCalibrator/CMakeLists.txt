################################################################################
# Package: MuonPatternCalibrator
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternCalibrator )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

atlas_add_library( MuonPatternCalibratorLib
                   MuonPatternCalibrator/*.h
                   INTERFACE
                   PUBLIC_HEADERS MuonPatternCalibrator
                   LINK_LIBRARIES GaudiKernel AthenaBaseComps MuonSegmentMakerToolInterfaces MuonRecToolInterfaces MuonPattern MuonPrepRawData MuonIdHelpersLib StoreGateLib MuonSegment Identifier )

# Component(s) in the package:
atlas_add_component( MuonPatternCalibrator
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} MuonPatternCalibratorLib MuonReadoutGeometry MuonRIO_OnTrack MuonRecHelperToolsLib MuonEDM_AssociationObjects TrkParameters TrkSegment TrkToolInterfaces )
