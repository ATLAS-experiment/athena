/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_ALIGNTRACKPREPROCESSOR_H
#define TRKALIGNGENTOOLS_ALIGNTRACKPREPROCESSOR_H


#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkAlignInterfaces/IAlignTrackPreProcessor.h"
#include "InDetAlignGenTools/IInDetAlignHitQualSelTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkFitterInterfaces/IGlobalTrackFitter.h"

#include <vector>

/**
   @file AlignTrackPreProcessor.h
   @class AlignTrackPreProcessor
   
   @brief Tool used to create AlignTracks from an input collection of tracks.
   At a minimum, the track is refit and the full covariance matrix and derivative
   matrix are stored on the AlignTrack.
   The AlignTracks are filled into a collection of Tracks.

   @author Robert Harrington <roberth@bu.edu>, Daniel Kollar <daniel.kollar@cern.ch>
   @date 10/1/09
*/


namespace Trk {

  class Track;
  class AlignTrackPreProcessor : virtual public IAlignTrackPreProcessor, public AthAlgTool
  {

  public:
    AlignTrackPreProcessor(const std::string & type, const std::string & name, const IInterface * parent);
    
    StatusCode initialize();
    StatusCode finalize();
    
    /** creates AlignTrack containing all TSOS on track */
    DataVector<Track> * processTrackCollection(const DataVector<Track>* trks);
   

  private:
    ToolHandle<IGlobalTrackFitter> m_trackFitterTool{
      this, "TrackFitterTool", "Trk::GlobalChi2Fitter/InDetTrackFitter"};
    ToolHandle<IGlobalTrackFitter> m_SLTrackFitterTool{
      this, "SLTrackFitterTool", ""};

    ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelectorTool{
      this, "TrackSelectorTool", ""};
    ToolHandle<IInDetAlignHitQualSelTool> m_hitQualityTool{
      this, "HitQualityTool", ""};
    
    /** select silicon hits by quality. keep all the rest **/
    Track * performSiliconHitSelection(const Track *, const ToolHandle<Trk::IGlobalTrackFitter> &);

    BooleanProperty m_refitTracks{this, "RefitTracks", true, "flag to refit tracks"};
    BooleanProperty m_storeFitMatricesAfterRefit{
      this, "StoreFitMatricesAfterRefit", true,
      "flag to store derivative and covariance matrices after refit"};

    BooleanProperty m_runOutlierRemoval{this, "RunOutlierRemoval", false,
      "run outlier removal in track refit"};
    IntegerProperty m_particleHypothesis{
      this, "ParticleHypothesis", Trk::nonInteracting,
      "particle hypothesis in track refit"};

    BooleanProperty m_useSingleFitter{this, "UseSingleFitter", false,
      "only use 1 fitter for refitting track"};

    BooleanProperty m_selectTracks{this, "SelectTracks", false,
      "do the track selection"};
    BooleanProperty m_selectHits{this, "SelectHits", false,
      "perform the hit InnerDetector selection"};
    BooleanProperty m_fixMomentum{this, "FixMomentum", false,
      "Fix the momentum of the track so it is not refitted"};

  }; // end class

} // end namespace



#endif // TRKALIGNGENTOOLS_ALIGNTRACKPREPROCESSOR_H


