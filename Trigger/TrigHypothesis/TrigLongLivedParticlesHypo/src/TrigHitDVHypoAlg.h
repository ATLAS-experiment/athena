/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGLONGLIVEDPARTICLESHYPO_TRIGHITDVHYPOALG_H
#define TRIGLONGLIVEDPARTICLESHYPO_TRIGHITDVHYPOALG_H

#include <string>

#include "Gaudi/Property.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "CxxUtils/checker_macros.h"
#include "DecisionHandling/HypoBase.h"
#include "TrigHitDVHypoTool.h"

#include "xAODJet/JetContainer.h"
#include "LumiBlockData/LuminosityCondData.h"
#include "LumiBlockComps/ILumiBlockMuTool.h"

#include "TMVA/Reader.h"

#include "TrigInDetToolInterfaces/ITrigSpacePointConversionTool.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "TrigInDetEvent/TrigSiSpacePointBase.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

/**
 * @class TrigHitDVHypoAlg
 * @brief Implements Hypo selection on triggering displaced vertex
 * @author Kunihiro Nagano <kunihiro.nagano@cern.ch> - KEK
 **/

struct HitDVSeed {
  float eta;
  float phi;
  int16_t type;
};

struct HitDVTrk {
  int      id;
  float    pt;
  float    eta;
  float    phi;
  int16_t  n_hits_inner;
  int16_t  n_hits_pix;
  int16_t  n_hits_sct;
  float    a0beam;
};

struct HitDVSpacePoint {
   float    eta;
   float    r;
   float    phi;
   int16_t  layer;
   bool     isPix;
   bool     isSct;
   int16_t  usedTrkId;
};

class TrigHitDVHypoAlg : public ::HypoBase
{
public:

   TrigHitDVHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );
   virtual StatusCode  initialize() override;
   virtual StatusCode  execute(const EventContext& context) const override;

private:

  ToolHandle<ITrigSpacePointConversionTool> m_spacePointTool{this, "SpacePointProviderTool", "TrigSpacePointConversionTool"};
  ToolHandleArray< TrigHitDVHypoTool >   m_hypoTools     {this, "HypoTools", {}, "Tools to perform selection"};

   // EDMs
   SG::ReadHandleKey< xAOD::JetContainer >          m_jetsKey      {this, "Jets",      "HLT_AntiKt4EMTopoJets_subjesIS", ""};
   SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_hitDVKey     {this, "HitDV",     "HLT_HitDV",     ""};
   SG::ReadHandleKey< TrackCollection>              m_tracksKey   {this, "HitDVTracks",   "HLT_IDTrkTrack_FS_FTF",   ""};

   // Luminosity related
   ToolHandle<ILumiBlockMuTool> m_lumiBlockMuTool;
   // Property: Per-bunch luminosity data (data only) conditions input)
   SG::ReadCondHandleKey<LuminosityCondData> m_lumiDataKey {this, "LuminosityCondDataKey", "LuminosityCondData", ""};
   // Property: MC flag.
   Gaudi::Property<bool> m_isMC {this, "isMC", false, "Real data or MC"};
   // Property: jet seed cut
   Gaudi::Property<float> m_jetSeed_ptMin  {this, "jetSeed_ptMin",  50.0, "Minimum pT  for jet seed"};
   Gaudi::Property<float> m_jetSeed_etaMax {this, "jetSeed_etaMin",  2.0, "Maximum eta for jet seed"};
   // Property: Name of the link to hitDV seed container to pass to ViewCreatorROITool
   Gaudi::Property<std::string>  m_hitDVLinkName {this, "hitDVLinkName", "HitDVSeedLink", "Name of the link to HitDVContainer. Used by ViewCreatorROITool."};

   // seed type enum
   using SeedType = TrigHitDVHypoTool::SeedType;

   // monitoring
   ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Monitoring tool" };
   StatusCode doMonitor(const xAOD::TrigCompositeContainer*) const;

   //
   float      deltaR2(float, float, float, float) const;
   int        getSPLayer(int, float) const;
   StatusCode findSPSeeds(const EventContext&, const std::vector<HitDVSpacePoint>&, std::vector<float>&, std::vector<float>&) const;
   StatusCode findJetSeeds(const xAOD::JetContainer*, const float, const float, std::vector<float>&, std::vector<float>&, std::vector<float>&) const;
   StatusCode selectSeedsNearby(const std::vector<HitDVSeed>& hitDVSeedsContainer,
				std::vector<float>& jetSeeds_eta, std::vector<float>& jetSeeds_phi, std::vector<float>& jetSeeds_pt) const;
   StatusCode calculateBDT(const EventContext&, const std::vector<HitDVSpacePoint>&, const std::vector<HitDVTrk>&,
			   const std::vector<float>&, const std::vector<float>&, const std::vector<float>&,
			   const float&, const int, xAOD::TrigCompositeContainer*, int&) const;

   // BDT
   struct TMVAReader {
      std::unique_ptr<TMVA::Reader> tmva_0eta1;
      std::unique_ptr<TMVA::Reader> tmva_1eta2;
      float n_track_qual;
      float ly0_sp_frac;
      float ly1_sp_frac;
      float ly2_sp_frac;
      float ly3_sp_frac;
      float ly4_sp_frac;
      float ly5_sp_frac;
      float ly6_sp_frac;
      float ly7_sp_frac;
   };
   mutable SG::SlotSpecificObj<TMVAReader> m_tmva_reader ATLAS_THREAD_SAFE;

   // parameters
   int m_tools_lowest_jetEt = 1000;
   int m_tools_loosest_wp = 0;

   //Ctrl Flags
   bool m_useBeamSpot = true;

   //Vertex
   bool m_doHitDV_Seeding = true;

   SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_jetRoiCollectionKey {this, "jFexSRJetRoI", "", ""};
   SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

   StatusCode findSPSeeds( const EventContext& ctx,
					     const std::vector<float>& v_sp_eta, const std::vector<float>& v_sp_phi,
					     const std::vector<int>& v_sp_layer, const std::vector<int>& v_sp_usedTrkId,
					     std::vector<float>& seeds_eta, std::vector<float>& seeds_phi ) const;
   StatusCode findHitDV(const EventContext& ctx, const std::vector<TrigSiSpacePointBase>& convertedSpacePoints,
					  const DataVector<Trk::Track>& tracks, std::vector<HitDVSeed>& hitDVSeedsContainer, 
                 std::vector<HitDVTrk>& hitDVTrksContainer,
                 std::vector<HitDVSpacePoint>& hitDVSPContainer) const;
};

#endif //> !TRIGLONGLIVEDPARTICLESHYPO_TRIGHITDVHYPOALG_H
