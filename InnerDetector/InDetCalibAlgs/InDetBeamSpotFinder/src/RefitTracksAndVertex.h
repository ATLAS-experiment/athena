/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// @file RefitTracksAndVertex.cxx
//

//
// $Id: RefitTracksAndVertex.h,v 1.8 2009-05-18 15:48:05 moles Exp $
 
#ifndef RefitTracksAndVertex_H
#define RefitTracksAndVertex_H

#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"

class AtlasDetectorID ;

namespace Trk {
  class Track ;
  class TrackStateOnSurface ;
  class PseudoMeasurementOnTrack ;
}

class RefitTracksAndVertex: public AthAlgorithm {
 public:
  /** The RefitTracksAndVertex is an implementation to add the so-called TRT momentum constraint on a track.
      It was developed in the context of alignment, and its main idea is that if the TRT is free from "weak modes",
      its curvature measurement can be added as a PseudoMeasurement (PM) to a Si-only track. This extra information,
      taken into account while refitting the Si-only part of the track or read in by the alignment algorithms
      directly, can serve to eliminate the alignment weak modes from the Si tracker of ATLAS.
      More details can be found at: https://twiki.cern.ch/twiki/bin/view/Atlas/TRTMomConstraint */
  using AthAlgorithm::AthAlgorithm;
  StatusCode initialize() ; //!< initialize method of this algorithm.   
  StatusCode execute(const EventContext& ctx)    ; //!< execute method of this algorithm that is called for each event  
  StatusCode finalize()   ; //!< finalize method of this algorithm. Prints out a summary of all events
 
  MsgStream& dump( MsgStream& outst ) const ;

 private:
  // helper functions
  /** Verifies if the given track passes the track selection criteria specified via the jobOptions */
  bool accept( const Trk::Track& track ) ;
  /** adds a PseudoMeasurement to a MeasurementSet */
  const Trk::MeasurementSet addPM( Trk::MeasurementSet& ms, const Trk::PseudoMeasurementOnTrack* pm ) ;
  /** creates a PseudoMeasurement with (z0, theta) from extended track perigee parameters */
  const Trk::PseudoMeasurementOnTrack* createPMfromSi ( const Trk::Perigee* mp ) ;
   /** Strips of all TRT hits from the track and replaces them with a TRT momentum constraint 
  from a the TRT segment belonging to the (extended) track (returns NULL in case of failure) */
  Trk::Track* fitSCTOnlyTrack( const Trk::Track* track ) ;

  // infrastructure  
  const AtlasDetectorID*  m_idHelper{nullptr}; //!< Detector ID helper
  Gaudi::Property<std::string> m_vertexListInput{this, "VertexListInput", "PrimaryVertices", "Name of the TrackCollection (input)"};
  Gaudi::Property<std::string> m_trackListOutput{this, "TrackListOutput", "SiTracksWithRefitTracksAndVertex", "Name of the TrackCollection (Output)"};
  Gaudi::Property<std::string> m_outputVertexContainerName{this, "OutputVertexContainer", "RefittedPrimaryVertex", "Name of vertex container"};
  PublicToolHandle<Trk::ITrackFitter> m_trackFitter{this, "TrackFitter", "Trk::GlobalChi2Fitter/InDetTrackFitter", "The TrackFitter to refit the tracks (segment, momentum constraint)"};
  PublicToolHandle<Trk::IVertexFitter> m_vertexFitter{this, "VertexFitterTool", "Trk::FullVertexFitter", "The VertexFitter to refit the vertices"};

  // algorithm-internal members
  Gaudi::Property<bool> m_applyTrkSel{this, "ApplyTrkSel", false, "apply a selection on tracks or not"};
  Gaudi::Property<bool> m_refitTracks{this, "RefitTracks", true, "refitTracks"};
  Gaudi::Property<bool> m_addPM{this, "AddPseudoMeasurement", false, "apply a pseudo measurement based on the original track (theta,z0)"};
  Gaudi::Property<double> m_selPtMin{this, "SelPtMin", 0, "minimal pT cut value for the TrackSelection"};
  Gaudi::Property<double> m_selEtaMin{this, "SelEtaMin", -2.5, "minimal eta cut value for the TrackSelection"};
  Gaudi::Property<double> m_selEtaMax{this, "SelEtaMax", 2.5, "maximal eta cut value for the TrackSelection"};
  Gaudi::Property<int> m_selNHitPIXMin{this, "SelNHitPIXMin", 0, "minimal number of PIX hits cut value for the TrackSelection"};
  Gaudi::Property<int> m_selNHitSCTMin{this, "SelNHitSCTMin", 6, "minimal number of SCT hits cut value for the TrackSelection"};
  size_t m_nTracksProcessed{0}; //!< Counter for number of tracks processed
  size_t m_nTracksPresel{0}; //!< Counter for number of tracks passing the preselection
  size_t m_nTracksAccepted{0}; //!< Counter for number of tracks passing the preselection and with PM
  size_t m_nRejectPIX{0}; //!< Counter for number of tracks failing the min number of PIX hits req.
  size_t m_nRejectSCT{0}; //!< Counter for number of tracks failing the min number of SCT hits req.
  size_t m_nRejectTRT{0}; //!< Counter for number of tracks failing the min number of TRT hits req.
  size_t m_nRejectPM{0}; //!< Counter for number of tracks failing the addition of a pseudo-measurement (PM)
} ;

MsgStream& operator<<( MsgStream& outst, const RefitTracksAndVertex& alg ) ;

#endif
