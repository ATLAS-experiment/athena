/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"

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
      BooleanProperty m_doRecoTrackCuts{this, "doRecoTrackCuts", false,
                                      "Apply Loose track cuts"};
      // eta dependent track selection
      ServiceHandle<IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc{
          this, "InDetEtaDependentCutsSvc", "InDetEtaDependentCutsSvc"};

      IntegerProperty m_minClusters{this, "minClusters", 6, "Min number clusters"};
      IntegerProperty m_minPixelClusters{this, "minPixelClusters", 1, "min pixel clusters"};
      IntegerProperty m_minStripClusters{this, "minStripClusters", 0,
                                    "Minimum number of strip clusters"};
      DoubleProperty m_pTmin{this, "pTmin", 400., "min pT"};
      DoubleProperty m_etamax{this, "etamax", 4., "max reco eta"};
      
      int passEtaDepCuts(const Trk::Track& track) const;

          
      std::tuple<bool, int, std::unique_ptr<Trk::Track>> doFitAndCut(
        const EventContext& ctx,
        std::vector<const Trk::SpacePoint*>& spacePoints,
        std::vector<const Trk::PrepRawData*>& clusters,
        int& trackCounter
      ) const;

      int passEtaDepCuts(const Trk::Track* track, int nClusters,
                                int nFreeClusters, int nPixels) const;

      bool prefitCheck(int nPix, int nStrip, int nClusters, int nSpacePoints) const;

      std::unique_ptr<Trk::Track> fitTrack(
        const EventContext& ctx,
        std::vector<const Trk::PrepRawData*> clusters,
        const Trk::TrackParameters& initial_params,
        int trackCounter
      ) const;

      std::vector<const Trk::SpacePoint*> getSpacePoints (
        const std::vector<uint32_t>& trackIndices,
        const std::vector<const Trk::SpacePoint*>& allSpacePoints
      ) const;

      std::vector<const Trk::PrepRawData*> spacePointsToClusters (
        const std::vector<const Trk::SpacePoint*>& spacePoints
      ) const;

      std::vector<const Trk::PrepRawData*> getClusters (
        const std::vector<std::vector<uint32_t>>& clusterTracks,
        const std::vector<const Trk::PrepRawData*>& allClusters,
        int trackNumber
      ) const;

      std::vector<const Trk::SpacePoint*> getSpacePointsInEvent (
        const EventContext& ctx,
        int eventNumber
      ) const;

      std::vector<const Trk::SpacePoint*> getSpacePointsInEvent(
        const EventContext& ctx,
        const SG::ReadHandleKey<SpacePointContainer>& containerKey
      ) const;

      std::vector<const Trk::SpacePoint*> getSpacePointsInEvent(
        const EventContext& ctx,
        const SG::ReadHandleKey<SpacePointOverlapCollection>& containerKey
      ) const ;

      std::vector<const Trk::PrepRawData*> getClustersInEvent (
        const EventContext& ctx,
        int eventNumber
      ) const;



    };
      
    MsgStream&    operator << (MsgStream&   ,const SiSPGNNTrackMaker&);
    std::ostream& operator << (std::ostream&,const SiSPGNNTrackMaker&); 
}

#endif
