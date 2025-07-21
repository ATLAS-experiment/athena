/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// filename: TrigFastTrackFinder.h
//
// Description: a part of HLT ID tracking
//
// date: 16/04/2013
//
// -------------------------------
// ATLAS Collaboration
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIGFASTTRACKFINDER_TRIGFASTTRACKFINDER_H
#define TRIGFASTTRACKFINDER_TRIGFASTTRACKFINDER_H



#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventUtils/PRDtoTrackMap.h"

#include "TrkToolInterfaces/ITrackSummaryTool.h"

#include "TrigInDetToolInterfaces/ITrigInDetTrackFitter.h"
#include "TrigInDetToolInterfaces/ITrigZFinder.h"

#include "InDetRecToolInterfaces/ISiTrackMaker.h"
#include "SiSPSeededTrackFinderData/SiTrackMakerEventData_xk.h"
#include "TrigInDetPattRecoTools/TrigCombinatorialSettings.h"


#include "AthenaMonitoringKernel/Monitored.h"

#include "TrkTrack/TrackCollection.h" //typedef, cannot fwd declare

#include "TrigInDetEvent/TrigVertexCollection.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"
#include "TrigInDetToolInterfaces/ITrigSpacePointConversionTool.h"
#include "TrigInDetToolInterfaces/ITrigInDetTrackSeedingTool.h"

// for UTT
#include "TrigT1Interfaces/RecJetRoI.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkFitterInterfaces/ITrackFitter.h"

//for GPU acceleration

#include "TrigInDetAccelerationTool/ITrigInDetAccelerationTool.h"
#include "TrigInDetAccelerationService/ITrigInDetAccelerationSvc.h"
#include "TrigAccelEvent/TrigInDetAccelCodes.h"

#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <memory>
#include <array>

namespace Trk {
  class Track; 
}
class SCT_ID;
class PixelID;
class TrigInDetTriplet;
class TrigSiSpacePointBase;

namespace InDet {
  class ExtendedSiTrackMakerEventData_xk : public InDet::SiTrackMakerEventData_xk
  {
  public:
    ExtendedSiTrackMakerEventData_xk(const SG::ReadHandleKey<Trk::PRDtoTrackMap> &key, const EventContext& ctx) {
      if (!key.key().empty()) {
        m_prdToTrackMap = SG::ReadHandle<Trk::PRDtoTrackMap>(key, ctx);
        if (!m_prdToTrackMap.isValid()) {
          throw std::runtime_error(std::string("Failed to get PRD to track map:") + key.key());
        }
        setPRDtoTrackMap(m_prdToTrackMap.cptr());
      }
    }
  private:
    void dummy() {}
    SG::ReadHandle<Trk::PRDtoTrackMap> m_prdToTrackMap;
  };
}

class TrigFastTrackFinder : public AthReentrantAlgorithm {

 public:

  TrigFastTrackFinder(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrigFastTrackFinder();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode start() override;

  virtual StatusCode execute(const EventContext& ctx) const override;

  StatusCode findTracks(InDet::SiTrackMakerEventData_xk &event_data,
                        const TrigRoiDescriptor& roi,
                        const TrackCollection* inputTracks,
                        TrackCollection& outputTracks,
                        const EventContext& ctx) const;

  double trackQuality(const Trk::Track* Tr) const;
  void filterSharedTracks(std::vector<std::tuple<bool, double, Trk::Track*>>& QT) const;

  enum DisTrkCategory
  {
     Other       = 0,
     Pix4l_Sct0  = 1,
     Pix4l_Sct1p = 2,
     Pix3l_Sct0  = 3,
     Pix3l_Sct1p = 4
  };

protected: 
  static constexpr size_t N_BARREL_LAYERS{8};//previously hardcoded in function
  struct OneLayerInfo_t{
    int nHits{};
    float chiSq{};
    int nDof{};
    int nGood{};
  };

  void updateClusterMap(long int, const Trk::Track*, std::map<Identifier, std::vector<long int> >&) const;
  void extractClusterIds(const Trk::SpacePoint*, std::vector<Identifier>&) const;
  bool usedByAnyTrack(const std::vector<Identifier>&, std::map<Identifier, std::vector<long int> >&) const;

 private:

  // AlgTools and Services

  ToolHandle<ITrigL2LayerNumberTool> m_numberingTool         {this, "LayerNumberTool", "TrigL2LayerNumberTool"}; 
  ToolHandle<ITrigSpacePointConversionTool> m_spacePointTool {this, "SpacePointProviderTool", "TrigSpacePointConversionTool"};
  ToolHandle<InDet::ISiTrackMaker> m_trackMaker;
  ToolHandle<ITrigInDetTrackFitter> m_trigInDetTrackFitter;
  ToolHandle<ITrigZFinder> m_trigZFinder;
  ToolHandle< Trk::ITrackSummaryTool > m_trackSummaryTool;
  ToolHandle< GenericMonitoringTool > m_monTool  { this, "MonTool", "", "Monitoring tool" };
  ToolHandle< Trk::IExtrapolator > m_extrapolator { this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator" };
  ToolHandle< Trk::ITrackFitter >  m_disTrkFitter;

  //for GPU acceleration
  ToolHandle<ITrigInDetAccelerationTool> m_accelTool {this, "TrigAccelerationTool", ""};
  ServiceHandle<ITrigInDetAccelerationSvc> m_accelSvc {this, "TrigAccelerationSvc", ""};

  //DataHandles
  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey;
  SG::ReadHandleKey<TrackCollection> m_inputTracksKey;
  SG::WriteHandleKey<TrackCollection> m_outputTracksKey;

  SG::ReadHandleKey<Trk::PRDtoTrackMap>       m_prdToTrackMap
     {this,"PRDtoTrackMap",""};

  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_dEdxTrkKey   {this, "dEdxTrk",   "", ""};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_dEdxHitKey   {this, "dEdxHit",   "", ""};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_disTrkCandKey{this, "DisTrkCand","", ""};

  // Control flags

  bool m_useBeamSpot;

  bool m_vertexSeededMode;

  bool m_doZFinder;
  bool m_doZFinderOnly;

  bool m_storeZFinderVertices;

  bool m_doFastZVseeding;

  bool m_doResMonitoring;

  // Cuts and settings
  TrigCombinatorialSettings m_tcs;

  int  m_minHits;

  int  m_nfreeCut;     // Min number free clusters


  float m_tripletMinPtFrac;
  float m_pTmin;
  float m_initialD0Max;
  float m_Z0Max;
  bool m_checkSeedRedundancy;

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  // Monitoring member functions

  void fillMon(const TrackCollection& tracks, const TrigVertexCollection& vertices, const TrigRoiDescriptor& roi, const EventContext& ctx) const;
  void runResidualMonitoring(const Trk::Track& track, const EventContext&) const;

  // Internal bookkeeping


  mutable std::atomic<unsigned int> m_countTotalRoI;
  mutable std::atomic<unsigned int> m_countRoIwithEnoughHits;
  mutable std::atomic<unsigned int> m_countRoIwithTracks;

  const PixelID* m_pixelId;
  const SCT_ID* m_sctId;
  const AtlasDetectorID* m_idHelper;


  Trk::ParticleHypothesis m_particleHypothesis;//particle hypothesis to attach to each track - usually pion, can be set to other values

  bool m_useNewLayerNumberScheme;


  // GPU acceleration

  bool m_useGPU;

  void makeSeedsOnGPU(const TrigCombinatorialSettings&, const IRoiDescriptor*, const std::vector<TrigSiSpacePointBase>&, std::vector<TrigInDetTriplet>&) const;

  // read input tracks and remove previously-used clusters from seed-making

  // Large Radius Tracking
  bool m_LRTmode;
  float m_LRTD0Min;
  float m_LRTHardMinPt;

  std::string m_trigseedML_LUT;//ML-based track seeding LUT name

  // create UTT EDMs (to avoid crash in case of findTrack's quitting before UTT algos)
  StatusCode createEmptyUTTEDMs(const EventContext&) const;

  // Hit-based displaced vertex
  int   getSPLayer(int, float) const;

  // dEdx calculation
  bool m_dodEdxTrk;
  StatusCode calcdEdx(const EventContext&, const TrackCollection&) const;
  StatusCode finddEdxTrk(const EventContext&, const TrackCollection&) const;
  float dEdx(const Trk::Track*, int&, int&, std::vector<float>&, std::vector<float>&,
	     std::vector<float>&, std::vector<float>&, std::vector<int>&, std::vector<int>&, std::vector<int>&) const;

  // disappearing track
  bool m_doDisappearingTrk;
  int recoAndFillDisTrkCand(const std::string&, TrackCollection*, std::vector<Trk::Track*>&, xAOD::TrigCompositeContainer*, 
			    const std::vector<double>&, const std::vector<double>&, const std::vector<double>&, bool, const EventContext&) const;
  void print_disTrk(const Trk::Track* t) const;
  std::unique_ptr<Trk::Track> disTrk_refit(Trk::Track* t, const EventContext& ctx) const;
  
  std::array<OneLayerInfo_t, N_BARREL_LAYERS> getTrkBarrelLayerInfo(Trk::Track* aTrack) const;
  bool isCleaningPassDisTrack(const TrigInDetTriplet&, Trk::Track*, bool) const;
  double disTrackQuality(const Trk::Track*) const;
  void recoVertexForDisTrack(const EventContext&, TrackCollection&, std::vector<double>&, std::vector<double>&, std::vector<double>&) const;
  bool isPreselPassDisTrackBeforeRefit(Trk::Track*, double, double) const;
  bool isPreselPassDisTrackAfterRefit(Trk::Track*, Trk::Track*, double, double) const;
  bool isGoodForDisTrackVertex(Trk::Track*, const EventContext&) const;
  std::unique_ptr<const Trk::TrackParameters> extrapolateDisTrackToBS(Trk::Track*, const std::vector<double>&, const std::vector<double>&, const std::vector<double>&, const EventContext&) const;
  void filterSharedDisTracks(std::vector<std::tuple<bool, double,Trk::Track*>>&) const;
  void fillDisTrkCand(xAOD::TrigComposite*, const std::string&, Trk::Track*, const std::unique_ptr<const Trk::TrackParameters>&) const;
  void fillDisTrkCand(xAOD::TrigComposite*, const std::string&, Trk::Track*, const std::unique_ptr<const Trk::TrackParameters>&, bool, std::vector<Trk::Track*>&) const;
  TrigFastTrackFinder::DisTrkCategory getDisTrkCategory(Trk::Track* trk) const;
  StatusCode findDisTracks(const EventContext&, TrackCollection&,
			   std::vector<std::tuple<bool, double, Trk::Track*>>&,
			   std::vector<std::tuple<bool, double, Trk::Track*>>&,
			   TrackCollection&,
			   const std::vector<double>&, const std::vector<double>&, const std::vector<double>&) const;

  // Phase II
  bool m_ITkMode;
  bool m_standaloneMode;

  ToolHandle<ITrigInDetTrackSeedingTool> m_seedingTool{this, "TrackSeedingTool", ""};
  
  Gaudi::Property<bool> m_useEtaBinning {this, "UseEtaBinning",   true, "Split layers into eta bins"};
  Gaudi::Property<bool> m_doCloneRemoval{this,  "doCloneRemoval", true, "Remove tracks sharing too many hits"};
  Gaudi::Property<bool> m_doTrackRefit  {this, "doTrackRefit",    true, "Refit tracks after the combinatorial track following"};
  Gaudi::Property<bool> m_useTracklets  {this, "UseTracklets",    false, "Use tracklet seeds from ITk track seeding"};
  Gaudi::Property<bool> m_useGBTSeedingTool {this, "useGBTSeedingTool", false, "GBT seeding tool for tracker"};
  
};


#endif // not TRIGFASTTRACKFINDER_TRIGFASTTRACKFINDER_H
