// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGAConversionAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include <type_traits>
#include <format>

FPGAConversionAlgorithm::FPGAConversionAlgorithm(const std::string& name, ISvcLocator* pSvcLocator ): 
  AthReentrantAlgorithm( name, pSvcLocator ){}

  StatusCode FPGAConversionAlgorithm::initialize() {
    ATH_CHECK(m_ClusterConverter.retrieve());

    ATH_CHECK(m_FPGAClusterKey.initialize(m_doClusters or m_doSP));
    ATH_CHECK(m_FPGASPKey.initialize(m_doSP));
    ATH_CHECK(m_FPGAHitKey.initialize(m_doHits));
    ATH_CHECK(m_FPGARoadKey.initialize(m_doActsTrk));
    ATH_CHECK(m_FPGAHitInRoadsKey.initialize(m_doActsTrk));
    ATH_CHECK(m_FPGATrackKey.initialize(m_doActsTrk));
    ATH_CHECK(m_xAODPixelClusterFromFPGAClusterKey.initialize(m_doClusters));
    ATH_CHECK(m_xAODStripClusterFromFPGAClusterKey.initialize(m_doClusters));
    ATH_CHECK(m_xAODPixelClusterFromFPGAHitKey.initialize(m_doHits));
    ATH_CHECK(m_xAODStripClusterFromFPGAHitKey.initialize(m_doHits));
    ATH_CHECK(m_xAODStripSpacePointFromFPGAKey.initialize(m_doSP));  
    ATH_CHECK(m_xAODPixelSpacePointFromFPGAKey.initialize(m_doSP));      
    ATH_CHECK(m_ActsProtoTrackFromFPGARoadKey.initialize(m_doActsTrk));
    ATH_CHECK(m_ActsProtoTrackFromFPGATrackKey.initialize(m_doActsTrk));
    ATH_CHECK(m_outputStripClusterContainerKey.initialize(m_doClusters));
    ATH_CHECK(m_outputPixelClusterContainerKey.initialize(m_doClusters));
    return StatusCode::SUCCESS;
  }

  StatusCode FPGAConversionAlgorithm::execute(const EventContext& ctx) const {
    std::unique_ptr<InDet::PixelClusterCollection> PixelCollFromHits = std::make_unique<InDet::PixelClusterCollection>(); 
    std::unique_ptr<InDet::SCT_ClusterCollection> SCTCollFromHits = std::make_unique<InDet::SCT_ClusterCollection>(); 

    std::unique_ptr<InDet::PixelClusterCollection> PixelCollFromClusters = std::make_unique<InDet::PixelClusterCollection>(); 
    std::unique_ptr<InDet::SCT_ClusterCollection> SCTCollFromClusters = std::make_unique<InDet::SCT_ClusterCollection>();

    std::unique_ptr<InDet::SCT_ClusterCollection> SCTCollFromSP = std::make_unique<InDet::SCT_ClusterCollection>();     
    std::unique_ptr<xAOD::PixelClusterContainer> PixelContFromHits = std::make_unique<xAOD::PixelClusterContainer>();
    std::unique_ptr<xAOD::PixelClusterAuxContainer> PixelAuxContFromHits = std::make_unique<xAOD::PixelClusterAuxContainer>();
    PixelContFromHits->setStore (PixelAuxContFromHits.get());

    std::unique_ptr<xAOD::StripClusterContainer> SCTContFromHits = std::make_unique<xAOD::StripClusterContainer>();
    std::unique_ptr<xAOD::StripClusterAuxContainer> SCTAuxContFromHits = std::make_unique<xAOD::StripClusterAuxContainer>();
    SCTContFromHits->setStore(SCTAuxContFromHits.get() );

    std::unique_ptr<xAOD::PixelClusterContainer> PixelContFromClusters = std::make_unique<xAOD::PixelClusterContainer>();
    std::unique_ptr<xAOD::PixelClusterAuxContainer> PixelAuxContFromClusters = std::make_unique<xAOD::PixelClusterAuxContainer>();
    PixelContFromClusters->setStore (PixelAuxContFromClusters.get());

    std::unique_ptr<xAOD::StripClusterContainer> SCTContFromClusters = std::make_unique<xAOD::StripClusterContainer>();
    std::unique_ptr<xAOD::StripClusterAuxContainer> SCTAuxContFromClusters = std::make_unique<xAOD::StripClusterAuxContainer>();
    SCTContFromClusters->setStore(SCTAuxContFromClusters.get() );

    std::unique_ptr<xAOD::SpacePointContainer> StripSPCont = std::make_unique<xAOD::SpacePointContainer>();
    std::unique_ptr<xAOD::SpacePointAuxContainer> StripSPAuxCont = std::make_unique<xAOD::SpacePointAuxContainer>();    
    StripSPCont->setStore(StripSPAuxCont.get() );

    std::unique_ptr<xAOD::SpacePointContainer> PixelSPCont = std::make_unique<xAOD::SpacePointContainer>();
    std::unique_ptr<xAOD::SpacePointAuxContainer> PixelSPAuxCont = std::make_unique<xAOD::SpacePointAuxContainer>();    
    PixelSPCont->setStore(PixelSPAuxCont.get() );
    
    std::unique_ptr<ActsTrk::ProtoTrackCollection> ProtoTracksFromRoads = std::make_unique<ActsTrk::ProtoTrackCollection>();
    std::unique_ptr<ActsTrk::ProtoTrackCollection> ProtoTracksFromTracks = std::make_unique<ActsTrk::ProtoTrackCollection>();

    clock_type::time_point startTime, stopTime;
    if (m_doClusters) {
      
      SG::ReadHandle<FPGATrackSimClusterCollection> FPGAClustersHandle (m_FPGAClusterKey, ctx);
      if (FPGAClustersHandle.isValid()) { // To avoid running over events that didn't pass truth tracks selections
        const FPGATrackSimClusterCollection *FPGAClusterColl = FPGAClustersHandle.cptr();

        // Convert to InDet clusters
        if (m_doIndet) {
          ATH_MSG_DEBUG("InDet Clusters CONVERSION");
          ATH_CHECK(m_ClusterConverter->convertClusters(*FPGAClusterColl, *PixelCollFromClusters, *SCTCollFromClusters));
        }
	  
        // Convert to xAOD clusters
        ATH_MSG_INFO("xAOD Clusters CONVERSION");
        startTime = clock_type::now();
        ATH_CHECK( m_ClusterConverter->convertClusters(*FPGAClusterColl, *PixelContFromClusters, *SCTContFromClusters) );
        stopTime = clock_type::now();
        m_totalClusterConversionTime += std::chrono::duration_cast<std::chrono::nanoseconds>(stopTime - startTime);

        if (m_doActsTrk) {
          SG::ReadHandle<FPGATrackSimRoadCollection> FPGARoadsHandle (m_FPGARoadKey, ctx);
          if (!FPGARoadsHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve 1st stage FPGATrackSimRoadCollection");
            return StatusCode::FAILURE;
          }

          const FPGATrackSimRoadCollection *FPGARoadColl = FPGARoadsHandle.cptr();

          SG::ReadHandle<FPGATrackSimHitContainer> FPGAHitsInRoadsHandle (m_FPGAHitInRoadsKey, ctx);
          if (!FPGAHitsInRoadsHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve 1st stage FPGATrackSimItInRoadCollection");
            return StatusCode::FAILURE;
          }
          const FPGATrackSimHitContainer *FPGAHitsInRoadsCont = FPGAHitsInRoadsHandle.cptr();

          SG::ReadHandle<FPGATrackSimTrackCollection> FPGATracksHandle (m_FPGATrackKey, ctx);
          if (!FPGATracksHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve 1st stage FPGATrackSimTrackCollection");
            return StatusCode::FAILURE;
          }
          const FPGATrackSimTrackCollection *FPGATrackColl = FPGATracksHandle.cptr();

          if (PixelContFromClusters->size()+SCTContFromClusters->size() > 0) {
            ATH_CHECK(m_ActsTrkConverter->findProtoTracks(ctx,*PixelContFromClusters,*SCTContFromClusters,*ProtoTracksFromRoads, *FPGAHitsInRoadsCont, *FPGARoadColl ));
            ATH_CHECK(m_ActsTrkConverter->findProtoTracks(ctx,*PixelContFromClusters,*SCTContFromClusters,*ProtoTracksFromTracks, *FPGATrackColl ));
          }
        }

        if (m_doSP) {
          ATH_MSG_INFO("Starting xAOD SpacePoint Conversion");
          SG::ReadHandle<FPGATrackSimClusterCollection> FPGASPHandle (m_FPGASPKey, ctx);

          if (FPGASPHandle.isValid()) { // To avoid running over events that didn't pass truth tracks selections
            const FPGATrackSimClusterCollection *FPGASPColl = FPGASPHandle.cptr();
            startTime = clock_type::now();
            ATH_CHECK( m_ClusterConverter->convertSpacePoints(*FPGASPColl, *StripSPCont, *PixelSPCont, *SCTContFromClusters, *PixelContFromClusters) );
            stopTime = clock_type::now();
            m_totalSpConversionTime += std::chrono::duration_cast<std::chrono::nanoseconds>(stopTime - startTime);
          }
          else {{ATH_MSG_WARNING("Failed to retrieve 1st stage FPGATrackSimSpacePointCollection. Will skip SP conversion ");}}
        }
      }

      else {ATH_MSG_WARNING("Failed to retrieve 1st stage FPGATrackSimClusterCollection. Will skip clusters and track conversion ");}
      SG::WriteHandle<xAOD::PixelClusterContainer> xAODPixelClusterFromFPGAClusterHandle (m_xAODPixelClusterFromFPGAClusterKey, ctx);
      SG::WriteHandle<xAOD::StripClusterContainer> xAODStripClusterFromFPGAClusterHandle (m_xAODStripClusterFromFPGAClusterKey, ctx);

      ATH_CHECK( xAODPixelClusterFromFPGAClusterHandle.record (std::move(PixelContFromClusters), std::move(PixelAuxContFromClusters)));
      ATH_CHECK( xAODStripClusterFromFPGAClusterHandle.record (std::move(SCTContFromClusters), std::move(SCTAuxContFromClusters)));
      if(m_doActsTrk)
      {
        SG::WriteHandle<ActsTrk::ProtoTrackCollection> ActsProtoTrackFromFPGARoadHandle (m_ActsProtoTrackFromFPGARoadKey, ctx);
        SG::WriteHandle<ActsTrk::ProtoTrackCollection> ActsProtoTrackFromFPGATrackHandle (m_ActsProtoTrackFromFPGATrackKey, ctx);
        ATH_CHECK( ActsProtoTrackFromFPGARoadHandle.record (std::move(ProtoTracksFromRoads)));
        ATH_CHECK( ActsProtoTrackFromFPGATrackHandle.record (std::move(ProtoTracksFromTracks)));
      }
      if (m_doSP) 
      {
        SG::WriteHandle<xAOD::SpacePointContainer> xAODStripSpacePointFromFPGAHandle (m_xAODStripSpacePointFromFPGAKey, ctx);
        SG::WriteHandle<xAOD::SpacePointContainer> xAODPixelSpacePointFromFPGAHandle (m_xAODPixelSpacePointFromFPGAKey, ctx);
        ATH_CHECK( xAODStripSpacePointFromFPGAHandle.record (std::move(StripSPCont), std::move(StripSPAuxCont)));
        ATH_CHECK( xAODPixelSpacePointFromFPGAHandle.record (std::move(PixelSPCont), std::move(PixelSPAuxCont)));
      }
    }  

    if (m_doHits) {

      SG::ReadHandle<FPGATrackSimHitCollection> FPGAHitsHandle (m_FPGAHitKey, ctx);

      if (FPGAHitsHandle.isValid()) {
        const FPGATrackSimHitCollection *FPGAHitColl = FPGAHitsHandle.cptr();

        // Convert to InDet clusters
        if (m_doIndet) {
          ATH_MSG_DEBUG("InDet Hits CONVERSION");
          ATH_CHECK(m_ClusterConverter->convertHits(*FPGAHitColl, *PixelCollFromHits, *SCTCollFromHits));
        }
        ATH_MSG_DEBUG("xAOD Hits CONVERSION");
        ATH_CHECK( m_ClusterConverter->convertHits(*FPGAHitColl, *PixelContFromHits, *SCTContFromHits) );
      }

      else {ATH_MSG_WARNING("Failed to retrieve 1st stage FPGATrackSimHitCollection. Will skip hit conversion ");}


      SG::WriteHandle<xAOD::PixelClusterContainer> xAODPixelClusterFromFPGAHitHandle (m_xAODPixelClusterFromFPGAHitKey, ctx);
      SG::WriteHandle<xAOD::StripClusterContainer> xAODStripClusterFromFPGAHitHandle (m_xAODStripClusterFromFPGAHitKey, ctx);
      ATH_CHECK( xAODPixelClusterFromFPGAHitHandle.record (std::move(PixelContFromHits),std::move(PixelAuxContFromHits)));
      ATH_CHECK( xAODStripClusterFromFPGAHitHandle.record (std::move(SCTContFromHits),std::move(SCTAuxContFromHits)));
    }

    m_nEvents++;
    return StatusCode::SUCCESS;
  }


template <typename T> // should be InDet::PixelCluster or InDet::SCT_Cluster
StatusCode FPGAConversionAlgorithm::convertCollectionToContainer(Trk::PrepRawDataCollection<T>* inputCollection, // i.e. InDet::PixelClusterCollection or InDet::SCT_ClusterCollection
                                                          SG::WriteHandleKey<Trk::PrepRawDataContainer<Trk::PrepRawDataCollection<T>>> &outputContainerKey
                                                          )
{
  if constexpr (!std::is_same<T, InDet::PixelCluster>::value && !std::is_same<T, InDet::SCT_Cluster>::value) {
    static_assert(false, "Bad type <T>. Should be InDet::PixelCluster or InDet::SCT_Cluster");
    return StatusCode::FAILURE;
  }

  std::unique_ptr<Trk::PrepRawDataContainer<Trk::PrepRawDataCollection<T>>> outputContainer = std::make_unique<Trk::PrepRawDataContainer<Trk::PrepRawDataCollection<T>>>(SG::VIEW_ELEMENTS);
  SG::WriteHandle<Trk::PrepRawDataContainer<Trk::PrepRawDataCollection<T>>> outputContainerHandle(outputContainerKey);
  
  if (!inputCollection || inputCollection->empty()){
    outputContainerHandle.record(std::move(outputContainer));
    return StatusCode::SUCCESS;
  }

  Trk::PrepRawDataCollection<T>* newCol = new Trk::PrepRawDataCollection<T>(inputCollection->identifyHash());
  newCol->setIdentifier(inputCollection->identify());
  for (const T* clus : *inputCollection) {
      newCol->push_back(std::make_unique<T>(*clus));
      ATH_MSG_DEBUG("NewCol cluster identifier " << clus->identify());
  }

  ATH_MSG_DEBUG("newCol->identifyHash() " << newCol->identifyHash());
  ATH_CHECK(outputContainer->addCollection(newCol,newCol->identifyHash()));
  
  if(outputContainerHandle.record(std::move(outputContainer)).isFailure()){
    ATH_MSG_ERROR("Could not record InDetSCTClusterContainer object with " 
		  << outputContainerHandle.key() 
		  << " into Event Store");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode FPGAConversionAlgorithm::finalize()
{
  std::string printoutStats = std::format(
    "\n|--------------------------------------------------|"
    "\n|          Process        |   avg. time per event  |"
    "\n|--------------------------------------------------|"
    "\n| xAOD cluster conversion |  {:>17.3f} sec |"
    "\n|      xAOD SP conversion |  {:>17.3f} sec |"
    "\n|--------------------------------------------------|",
    1e-9 * m_totalClusterConversionTime.count()/m_nEvents,
    1e-9 * m_totalSpConversionTime.count()/m_nEvents
  );

  ATH_MSG_INFO("Runtime stats:" + printoutStats);

  return StatusCode::SUCCESS;
}
