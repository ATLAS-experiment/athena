# Copyright (C) 2002-2026 by CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonTrackSteeringTools )

# Component(s) in the package:
atlas_add_component( MuonTrackSteeringTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel CxxUtils GeoPrimitives Identifier EventPrimitives GaudiKernel MuonReadoutGeometry MuPatPrimitives FourMomUtils
			            MuonIdHelpersLib MuonStationIndexLib MuonCompetingRIOsOnTrack MuonPrepRawData MuonRIO_OnTrack MuonSegment MuonRecHelperToolsLib MuonRecToolInterfaces MuonSegmentMakerUtils 
			 	    TrkDetDescrUtils TrkGeometry TrkSurfaces TrkEventPrimitives TrkMeasurementBase TrkParameters TrkPseudoMeasurementOnTrack TrkSegment TrkTrack TrkTrackSummary 
				    TrkExInterfaces TrkExUtils TrkFitterInterfaces TrkToolInterfaces TrkDriftCircleMath MagFieldConditions MuonSegmentMakerToolInterfaces MuonTrackMakerUtils )
