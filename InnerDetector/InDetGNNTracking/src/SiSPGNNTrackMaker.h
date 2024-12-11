/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SiSPGNNTrackMaker_H
#define SiSPGNNTrackMaker_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/DataHandle.h"

// data containers
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "TrkTrack/TrackCollection.h"

// Tool handles
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"
#include "InDetRecToolInterfaces/ISeedFitter.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "IGNNTrackReaderTool.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"

namespace Trk {
  class ITrackFitter;
}


namespace InDet {
   /**
   * @class InDet::SiSPGNNTrackMaker
   * 
   * @brief InDet::SiSPGNNTrackMaker is an algorithm that uses the GNN-based 
   * track finding tool to reconstruct tracks and the use track fitter to obtain
   * track parameters. It turns a collection of Trk::Tracks.
   * 
   * @author xiangyang.ju@cern.ch
   */
    class SiSPGNNTrackMaker : public AthReentrantAlgorithm {
      public:
      SiSPGNNTrackMaker(const std::string& name, ISvcLocator* pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

      /// Make this algorithm clonable.
      virtual bool isClonable() const override { return true; };
      //@}

      MsgStream&    dump     (MsgStream&    out) const;
      std::ostream& dump     (std::ostream& out) const;

      protected:
      /// --------------------
      /// @name Data handles
      /// --------------------
      //@{
      // input containers
      SG::ReadHandleKey<SpacePointContainer> m_SpacePointsPixelKey{
        this, "SpacePointsPixelName", "ITkPixelSpacePoints"};
      SG::ReadHandleKey<SpacePointContainer> m_SpacePointsSCTKey{
        this, "SpacePointsSCTName", "ITkStripSpacePoints"};
      SG::ReadHandleKey<SpacePointOverlapCollection> m_SpacePointsOverlapKey{this, "SpacePointsOverlapName", "ITkOverlapSpacePoints"};
      //@}
  SG::ReadHandleKey<InDet::PixelClusterContainer> m_ClusterPixelKey{
      this, "PixelClusterContainer", "ITkPixelClusters"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_ClusterStripKey{
      this, "StripClusterContainer", "ITkStripClusters"};

      // output container
      SG::WriteHandleKey<TrackCollection> m_outputTracksKey{
        this, "TracksLocation", "SiSPGNNTracks"};

      /// --------------------
      /// @name Tool handles
      /// --------------------
      //@{
      /// GNN-based track finding tool that produces track candidates
      ToolHandle<IGNNTrackFinder> m_gnnTrackFinder{
        this, "GNNTrackFinderTool", 
        "InDet::SiGNNTrackFinderTool", "Track Finder"
      };
      ToolHandle<ISeedFitter> m_seedFitter{
        this, "SeedFitterTool",
        "InDet::SiSeedFitterTool", "Seed Fitter"
      };
      /// Track Fitter
      ToolHandle<Trk::ITrackFitter> m_trackFitter {
        this, "TrackFitter", 
        "Trk::GlobalChi2Fitter/InDetTrackFitter", "Track Fitter"
      };
      ToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{
      this, "TrackSummaryTool", "InDetTrackSummaryTool"};
      ToolHandle<IGNNTrackReaderTool> m_gnnTrackReader{
        this, "GNNTrackReaderTool",
        "InDet::GNNTrackReaderTool", "Track Reader"
      };
      //@}

      MsgStream&    dumptools(MsgStream&    out) const;
      MsgStream&    dumpevent(MsgStream&    out) const;
      BooleanProperty m_areInputClusters{
        this, "areInputClusters", false,
        "Read track candidates as list of clusters"};
    };

    MsgStream&    operator << (MsgStream&   ,const SiSPGNNTrackMaker&);
    std::ostream& operator << (std::ostream&,const SiSPGNNTrackMaker&); 
}

#endif
