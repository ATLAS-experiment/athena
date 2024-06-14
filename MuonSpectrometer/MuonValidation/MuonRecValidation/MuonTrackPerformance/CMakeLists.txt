# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonTrackPerformance )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( MuonTrackPerformance
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AtlasHepMCLib AthenaBaseComps CxxUtils GeneratorObjects xAODEventInfo GaudiKernel MuonIdHelpersLib MuonStationIndexLib MuonSegment MuonRecHelperToolsLib MuonRecToolInterfaces TrkParameters TrkTrack TrkTrackSummary TrkTruthData TrkToolInterfaces TruthUtils StoreGateLib EventPrimitives xAODMuon xAODTracking xAODTruth MuonReadoutGeometry MuonClusterizationLib MuonCompetingRIOsOnTrack MuonLayerEvent MuonPrepRawData MuonRIO_OnTrack MuonSimData MuonLayerHough MuonRecValidationNtuples MuonCombinedEvent TrkMeasurementBase TrkPrepRawData TrkExInterfaces TrackRecordLib MuonSegmentMakerToolInterfaces MuonSegmentTaggerToolInterfaces )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


