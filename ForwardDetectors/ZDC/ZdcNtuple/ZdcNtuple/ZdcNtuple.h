/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZdcNtuple_ZdcNtuple_H
#define ZdcNtuple_ZdcNtuple_H

//#include <EventLoop/Algorithm.h>

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/AnaToolHandle.h>

#include <TrigDecisionTool/TrigDecisionTool.h>


#include "xAODEventInfo/EventInfo.h"
#include <AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h>
#include <TH1.h>
#include <TH2.h>
#include "xAODForward/MBTSModuleContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ForwardEventInfoContainer.h"
#include <TTree.h>
#include <xAODHIEvent/HIEventShapeContainer.h>
#include <xAODTracking/VertexContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTrigger/TrigDecision.h>
#include <xAODTrigger/EnergySumRoI.h>

#include <xAODTrigMinBias/TrigT2MbtsBitsContainer.h>
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "xAODForward/MBTSModuleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicRegistry.h"

#include "ZdcAnalysis/IZdcAnalysisTool.h"

#include "boost/unordered_map.hpp"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "xAODForward/AFPProtonContainer.h"
#include "xAODForward/AFPTrackContainer.h"
#include <TLorentzVector.h>
#include "StoreGate/ReadHandleKey.h"

class ZdcNtuple : public EL::AnaAlgorithm
{
public:
  bool slimmed; // assume slimmed output
  bool useGRL; // use a GRL (used by default)
  std::string grlFilename;
  bool enableOutputTree; // enable output TTree
  bool enableOutputSamples; // write samples into TTree
  bool enableTrigger; // use trigger info (skip this for laser runs)
  bool writeOnlyTriggers; // only write passed triggers to ntuple
  bool enableClusters; // store tracks
  bool enableTracks; // store clusters
  bool enableMuons; // store muons in ntuple
  bool enableElectrons; // store electrons in ntuple
  bool enablePhotons; // store photons in ntuple
  bool enableTT; // store trigger towers
  bool enableTruth; // store truth info
  bool enableJets; // store jets
  bool enableTriggerJets; // store trigger jets in ntuple
  bool zdcCalib; // run calibration file
  bool zdcLaser; // run laser file
  bool zdcOnly; // process only ZDC+trigger 
  bool zdcLowGainOnly; // ZDC LG only
  size_t trackLimit; // max tracks for writing tree
  bool trackLimitReject;
  bool flipDelay; // whether or not to flip un/delay for EM ZDC modules
  bool reprocZdc; // use ZdcAnalysisTool to reprocess ZDC, otherwise use what is there
  std::string auxSuffix; // what to add to name the new data, when reprocessing
  size_t nsamplesZdc; // nsamples expected by ZDC tool
  bool lhcf2022;
  bool lhcf2022zdc;
  bool lhcf2022afp;
  bool pbpb2023;
  bool enableRPD;
  bool enableCentroid;

  bool doZdcCalib;
  std::string zdcConfig;

  std::string outputName; // output for tree
  int outputTreeScaledown; // scaledown factor

public:

  asg::AnaToolHandle<Trig::TrigDecisionTool>  m_trigDecisionTool;
  asg::AnaToolHandle<IGoodRunsListSelectionTool> m_grl;
  asg::AnaToolHandle<ZDC::IZdcAnalysisTool> m_zdcAnalysisTool;
  ToolHandle< InDet::IInDetTrackSelectionTool > m_selTool;

  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
    { this, "ZdcModuleContainerName", "ZdcModules", "" };
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
    { this, "ZdcSumContainerName", "ZdcSums", "" };
  
  const xAOD::EventInfo* m_eventInfo;

  // Containers
  const xAOD::TrigDecision* m_trigDecision;
  const xAOD::HIEventShapeContainer *m_caloSums;
  const xAOD::HIEventShapeContainer *m_eventShapes;
  const xAOD::ForwardEventInfoContainer* m_mbtsInfo;
  const xAOD::MBTSModuleContainer* m_mbtsModules;
  const xAOD::TrigT2MbtsBitsContainer* m_trigT2MbtsBits;
  const xAOD::VertexContainer* m_primaryVertices;
  const xAOD::CaloClusterContainer* m_caloClusters;
  const xAOD::TrackParticleContainer* m_trackParticles;
  const xAOD::EnergySumRoI* m_lvl1EnergySumRoI;
  const xAOD::TruthParticleContainer* m_truthParticleContainer;
  const xAOD::TriggerTowerContainer* m_TTcontainer;
  const xAOD::AFPProtonContainer* m_afpProtons;


  int m_nTriggers;
  int m_eventCounter;
  bool m_isMC;
  bool m_setupTrigHist;
  int m_scaledownCounter;
  std::vector<const Trig::ChainGroup*> m_chainGroups;
  std::vector<const Trig::ChainGroup*> m_rerunChainGroups;

  float t_L1ET;
  float t_L1ET24;

  // Histograms
  void setupTriggerHistos();
  TH1* h_zdcTriggers;
  TH1* h_zdcTriggersTBP;

  TTree* m_outputTree;
  uint32_t t_runNumber;
  uint32_t t_eventNumber;
  uint32_t t_lumiBlock;
  uint32_t t_bcid;
  uint8_t t_bunchGroup;
  uint32_t t_passBits;
  uint32_t t_extendedLevel1ID;
  uint32_t t_timeStamp;
  uint32_t t_timeStampNSOffset;
  float t_avgIntPerCrossing;
  float t_actIntPerCrossing;
  uint8_t t_zdcEventInfoError;
  uint32_t t_zdcEventInfoErrorWord;

  uint64_t t_trigger;
  uint32_t t_trigger_TBP;
  //std::vector<uint16_t> t_decisions;
  //std::vector<float> t_prescales;
  float t_prescales[200];
  bool t_decisions[200];
  bool t_rerunDecisions[200];

  float t_mbts_in_e[2][8];
  float t_mbts_out_e[2][4];
  float t_mbts_in_t[2][8];
  float t_mbts_out_t[2][4];

  float t_T2mbts_in_e[2][8];
  float t_T2mbts_out_e[2][4];
  float t_T2mbts_in_t[2][8];
  float t_T2mbts_out_t[2][4];

  uint32_t t_tav[16];
  uint32_t t_tbp[16];

  float t_ZdcAmp[2];
  float t_ZdcAmpErr[2];
  float t_ZdcEnergy[2];
  float t_ZdcEnergyErr[2];
  float t_ZdcTime[2];
  short t_ZdcStatus[2];
  unsigned int t_ZdcModuleMask;
  float t_ZdcTrigEff[2];
  unsigned short t_ZdcLucrodTriggerSideAmp[2];

  float t_ZdcModuleAmp[2][4];
  float t_ZdcModuleTime[2][4];
  float t_ZdcModuleFitAmp[2][4];
  float t_ZdcModuleFitT0[2][4];
  float t_ZdcModuleChisq[2][4];
  unsigned int t_ZdcModuleStatus[2][4];
  float t_ZdcModuleCalibAmp[2][4];
  float t_ZdcModuleCalibTime[2][4];
  float t_ZdcModuleAmpError[2][4];
  float t_ZdcModuleBkgdMaxFraction[2][4];
  float t_ZdcModuleMinDeriv2nd[2][4];
  float t_ZdcModulePresample[2][4];
  float t_ZdcModulePreSampleAmp[2][4];
  unsigned short t_ZdcLucrodTriggerAmp[2][4];
  float t_ZdcModuleMaxADC[2][4];

  float t_RpdChannelBaseline[2][16];
  float t_RpdChannelPileupExpFitParams[2][16][2];
  float t_RpdChannelPileupStretchedExpFitParams[2][16][3];
  float t_RpdChannelPileupExpFitParamErrs[2][16][2];
  float t_RpdChannelPileupStretchedExpFitParamErrs[2][16][3];
  float t_RpdChannelPileupExpFitMSE[2][16];
  float t_RpdChannelPileupStretchedExpFitMSE[2][16];
  float t_RpdChannelAmplitude[2][16];
  float t_RpdChannelAmplitudeCalib[2][16];
  float t_RpdChannelMaxADC[2][16];
  float t_RpdChannelMaxADCCalib[2][16];
  unsigned int t_RpdChannelMaxSample[2][16];
  unsigned int t_RpdChannelStatus[2][16];
  float t_RpdChannelPileupFrac[2][16];
  unsigned int t_RpdSideStatus[2];

  bool t_centroidEventValid;
  unsigned int t_centroidStatus[2];
  float t_RPDChannelSubtrAmp[2][16];
  float t_RPDSubtrAmpSum[2];
  float t_xCentroidPreGeomCorPreAvgSubtr[2];
  float t_yCentroidPreGeomCorPreAvgSubtr[2];
  float t_xCentroidPreAvgSubtr[2];
  float t_yCentroidPreAvgSubtr[2];
  float t_xCentroid[2];
  float t_yCentroid[2];
  float t_xRowCentroid[2][4];
  float t_yColCentroid[2][4];
  float t_reactionPlaneAngle[2];
  float t_cosDeltaReactionPlaneAngle;

  int   t_nvx;
  float t_vx[3];
  int t_vxntrk;
  std::vector<int> t_vx_trk_index;
  int t_vxtype;
  int t_pvindex;
  float t_vxcov[6];
  float t_vxsumpt2;
  float t_puvxz;
  int t_puvxntrk;
  float t_puvxsumpt;
  int t_nstrong;
  int t_vxnlooseprimary;
  int t_vxnminbias;
  int t_vxngoodmuon;

  int t_nvtx;
  std::vector<int8_t> t_vtx_type;
  std::vector<float> t_vtx_x;
  std::vector<float> t_vtx_y;
  std::vector<float> t_vtx_z;
  std::vector<int16_t> t_vtx_ntrk_all;
  std::vector<float> t_vtx_sumpt2_all;
  std::vector<int16_t> t_vtx_ntrk;
  std::vector<float> t_vtx_sumpt2;
  std::vector< std::vector<int16_t> > t_vtx_trk_index;

  // FCal
  float t_fcalEt;
  float t_fcalEtA;
  float t_fcalEtC;
  float t_fcalEtA_TT;
  float t_fcalEtC_TT;
  float t_fcalEtA_TTsum;
  float t_fcalEtC_TTsum;

  float t_totalEt;
  float t_totalEt_TTsum;
  float t_totalEt24;
  float t_totalEt24_TTsum;

  // gaps
  float t_edgeGapA;
  float t_edgeGapC;
  float m_gapPtMin;

  float m_gapThresholds[98] =
    {4.7426,5.11018,5.07498,5.0969,5.10695,5.04098,5.07106,4.98087,5.11647,5.08988,5.16267,
     5.17202,5.23803,5.25314,5.29551,5.35092,5.40863,5.44375,5.38075,5.25022,5.37933,5.25459,5.37719,5.25169,5.73985,
     5.79174,5.79266,5.79588,5.7963,5.81949,5.82273,5.85658,5.85442,5.84779,5.77679,5.83323,5.84524,5.84439,5.84488,
     5.84744,5.84683,5.84524,5.84594,5.84656,5.84639,5.84461,5.84515,5.84206,5.8396,5.84497,5.84801,5.84608,5.84608,
     5.84783,5.84726,5.84844,5.8477,5.84796,5.84757,5.84822,5.84814,5.84617,5.83451,5.77658,5.84309,5.85496,5.85761,
     5.82555,5.82206,5.78982,5.78482,5.7778,5.78327,5.74898,5.25459,5.37503,5.25459,5.37283,5.25169,5.37862,5.44473,
     5.41041,5.34498,5.29551,5.25602,5.2283,5.17428,5.14504,5.09342,5.12256,4.98721,5.07106,5.02642,5.10031,5.11018,
     5.05447,5.10031,4.7426};
  TH1* h_TCSigCut;

  // mbts
  uint16_t t_mbts_countA;
  uint16_t t_mbts_countC;
  float t_mbts_timeA;
  float t_mbts_timeC;
  float t_mbts_timeDiff;

  uint16_t t_T2mbts_countAin;
  uint16_t t_T2mbts_countCin;

  uint16_t t_raw7[2][4][2][2][7];
  uint16_t t_raw15[2][4][2][2][15];
  uint16_t t_raw24[2][4][2][2][24];

  uint16_t t_rpdRaw[2][16][24];

  // tracks
  uint32_t t_ntrk;
  std::vector<float> t_trk_pt;
  std::vector<float> t_trk_eta;
  std::vector<float> t_trk_phi;
  std::vector<float> t_trk_e;
  std::vector<float> t_trk_theta;
  std::vector<float> t_trk_d0;
  std::vector<float> t_trk_z0;
  std::vector<float> t_trk_vz;
  std::vector<float> t_trk_vtxz;
  std::vector<int8_t> t_trk_charge;
  std::vector<int16_t> t_trk_quality;
  std::vector<int> t_trk_index;
  std::vector<uint8_t> t_trk_nPixHits;
  std::vector<uint8_t> t_trk_nSctHits;
  std::vector<uint8_t> t_trk_nPixDead;
  std::vector<uint8_t> t_trk_nSctDead;
  std::vector<uint8_t> t_trk_nPixHoles;
  std::vector<uint8_t> t_trk_nSctHoles;
  std::vector<uint8_t> t_trk_nTrtHits;
  std::vector<uint8_t> t_trk_nTrtOutliers;
  std::vector<uint8_t> t_trk_inPixHits;
  std::vector<uint8_t> t_trk_exPixHits;
  std::vector<uint8_t> t_trk_ninPixHits;
  std::vector<uint8_t> t_trk_nexPixHits;
  std::vector<float> t_trk_pixeldEdx;

  // clusters
  uint32_t t_nclus;
  std::vector<float> t_cc_pt;
  std::vector<float> t_cc_eta;
  std::vector<float> t_cc_phi;
  std::vector<float> t_cc_e;
  std::vector<float> t_cc_sig;
  std::vector<int> t_cc_layer;
  std::vector<float> t_cc_raw_m;
  std::vector<float> t_cc_raw_eta;
  std::vector<float> t_cc_raw_phi;
  std::vector<float> t_cc_raw_e;
  std::vector<std::vector<float>> t_cc_raw_samp;

  float t_clusEt;
  float t_clusEtMax;
  float t_clusetaMax;
  float t_clusphiMax;

  // AFP protons                                                                                                  

  int nProtons;
  std::vector<double> proton_pt;
  std::vector<double> proton_eta;
  std::vector<double> proton_phi;
  std::vector<double> proton_e;
  std::vector<int> proton_side;
  std::vector<double> proton_eLoss;
  std::vector<double> proton_t;
  std::vector<std::vector<int>> proton_track_stationID;
  std::vector<std::vector<float>> proton_track_xLocal;
  std::vector<std::vector<float>> proton_track_yLocal;
  std::vector<std::vector<float>> proton_track_zLocal;
  std::vector<std::vector<float>> proton_track_xSlope;
  std::vector<std::vector<float>> proton_track_ySlope;
  std::vector<std::vector<int>> proton_track_nClusters;

  TLorentzVector p_beam;
  TLorentzVector p_scat;

  // end of Histograms

  // this is a standard constructor
  ZdcNtuple (const std::string& name, ISvcLocator* pSvcLocator);

  void processEventInfo();
  bool processTriggerDecision();
  uint32_t acceptEvent();
  void processZdcNtupleFromModules(); // new version directly from output of ZdcAnalysisTool - which is much cleaner
  void processFCal();
  void processMBTS();
  void processInDet();
  void writeTrack(const xAOD::TrackParticle*, const xAOD::Vertex* vertex, int );
  int trackQuality(const xAOD::TrackParticle* tp, const xAOD::Vertex* vertex);
  void processClusters();
  void reprocessZdcModule(const xAOD::ZdcModule* zdcMod, bool flipdelay=0);
  void processTriggerTowers();
  void processGaps();
  void processProtons();

  double dR(const double eta1,     const double phi1,     const double eta2,     const double phi2);


  virtual StatusCode initialize () override;
  virtual StatusCode execute () override;
  virtual StatusCode finalize () override;
};

#endif
