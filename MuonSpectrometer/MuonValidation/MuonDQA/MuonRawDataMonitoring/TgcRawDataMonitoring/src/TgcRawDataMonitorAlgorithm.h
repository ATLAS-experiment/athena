/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCRAWDATAMONITORING_TGCRAWDATAMONITORALGORITHM_H
#define TGCRAWDATAMONITORING_TGCRAWDATAMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "MuonTrigCoinData/TgcCoinDataContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "GoodRunsLists/IGoodRunsListSelectorTool.h"
#include "ITgcRawDataMonitorTool.h"
#include "MuonPrepRawData/TgcPrepDataContainer.h"
#include "TrigConfData/L1Menu.h"
#include "xAODTracking/VertexContainer.h"
#include "TruthUtils/ParticleConstants.h"
#include <memory>
#include <vector>
#include <set>

class TgcRawDataMonitorAlgorithm : public AthMonitorAlgorithm {
 public:
  TgcRawDataMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TgcRawDataMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  enum TgcCoinPtBitShifts{
    CoinFlagF=4,CoinFlagC,CoinFlagH,CoinFlagEI,CoinFlagTile,CoinFlagRPC,CoinFlagNSW,
    CoinFlags = CoinFlagF, InnerCoinFlags = CoinFlagEI
  };
  struct TimedMuonRoI{
    int timing{}; // -2,-1,0(current BC),+1,+2
    const xAOD::MuonRoI* muonRoI{};
    TimedMuonRoI(const xAOD::MuonRoI* muonroi, int bcid = 0){
      timing = bcid;
      muonRoI = muonroi;
    }
  };
  struct TimedMuon{
    const xAOD::Muon* muon{};
    std::vector<double> extPosZ;
    std::vector<TVector3> extPos;
    std::vector<TVector3> extVec;
    std::set<int> matchedL1ThrExclusive;
    std::set<int> matchedL1ThrExclusiveTGC;
    std::set<int> matchedL1ThrInclusive;
    std::set<int> matchedL1ThrInclusiveTGC;
    std::set<std::string> matchedL1Items;
    bool matchedL1Charge{};
    bool passBW3Coin{};
    bool passInnerCoin{};
    bool passGoodMF{};
    bool passIsMoreCandInRoI{};
    void clear(){
      extPosZ.clear();
      extPos.clear();
      extVec.clear();
      matchedL1ThrExclusive.clear();
      matchedL1ThrExclusiveTGC.clear();
      matchedL1ThrInclusive.clear();
      matchedL1ThrInclusiveTGC.clear();
      matchedL1Items.clear();
    }
  };
  struct ExtPos{
    double extPosZ{};
    Amg::Vector3D extPos;
    Amg::Vector3D extVec;
    int matched{};
    const xAOD::Muon* muon{};
    std::set<std::string> passedChambers;
    std::set<std::string> chambersHasHit;
    std::map<std::string,std::set<double>> residuals;
  };
  struct ExtTrigInfo{
    double eta{};
    double phi{};
    double matched{};
    double matchedQ{};
    double matchedF{};
    double matchedC{};
    double matchedH{};
    double matchedEI{};
    double matchedTile{};
    double matchedRPC{};
    double matchedNSW{};
  };
  struct TgcTrigTile{
    int slSector{-999}; // SL sector ID
    double roiEta{-999}; // SL RoI at the current BC and with matched offline muon
    double roiPhi{-999};
    int roiNum{-999};
    int deltaBcid{-999}; // BCID difference between SL and TMDB
    int deltaTiming{-999}; // Signal timing difference between SL and TMDB
    int tmdbDecisions{-999}; // TMDB signal decisions for modules 0..3 (D5 only, D6 only, D5+D6)
    int bcid{-999};
    int bunch{-999};
    int currBc{0};
    int goodBcid0{0};
    int goodBcid1{0};
    int goodBcid2{0};
    int goodTiming{0};
  };
  struct TgcTrigNsw{
    int slSector{-999};
    int slInput{-999};
    int slInputIndex{-999};
    double roiEta{-999};
    double roiPhi{-999};
    int roiNum{-999};
    int isForward{-999};
    int isAside{-999};
    double deltaR{-999};
    int deltaBcid{-999}; // BCID difference between SL and NSW
    int deltaTiming{-999}; // Signal timing difference between SL and NSW
    int R{-999};
    int Phi{-999};
    int deltaTheta{-999};
    int bcid{-999};
    int bunch{-999};
    int currBc{0};
    int goodBcid0{0};
    int goodBcid1{0};
    int goodBcid2{0};
    int goodTiming{0};
    int bcmask{0};
  };
  struct TgcTrigRpc{
    int slSector{-999};
    double roiEta{-999};
    double roiPhi{-999};
    int roiNum{-999};
    int deltaBcid{-999};
    int deltaTiming{-999};
    int rpcEta{-999};
    int rpcPhi{-999};
    int rpcDEta{-999};
    int rpcDPhi{-999};
    int bcid{-999};
    int bunch{-999};
    int currBc{0};
    int goodBcid0{0};
    int goodBcid1{0};
    int goodBcid2{0};
    int goodTiming{0};
  };
  struct TgcTrigEifi{
    int slSector{-999};
    double roiEta{-999};
    double roiPhi{-999};
    int roiNum{-999};
    int deltaTiming{-999};
    int bunch{-999};
    int currBc{0};
    int goodTiming{0};
  };
  struct TgcTrig{
    int lb{};
    double x_In{};
    double y_In{};
    double z_In{};
    double x_Out{};
    double y_Out{};
    double z_Out{};
    double eta{};
    double phi{};
    double etain{};
    double etaout{};
    double width_In{};
    double width_Out{};
    double width_R{};
    double width_Phi{};
    int isAside{};
    int isForward{};
    int isStrip{};
    int isInner{};
    int isPositiveDeltaR{};
    int type{};
    int trackletId{};
    int trackletIdStrip{};
    int sector{};
    int roi{};
    int pt{};
    int delta{};
    int sub{};
    int veto{};
    int bunch{};
    int inner{};
    int muonMatched{};
    int loosemuonMatched{};
    int isBiased{};
    int bcid{};
    std::vector<TgcTrigTile*> tile;
    std::vector<TgcTrigNsw*> nsw;
    std::vector<TgcTrigNsw*> nsw_unique;
    std::vector<TgcTrigRpc*> rpc;
    std::vector<TgcTrigEifi*> eifi;
  };
  struct CtpDecMonObj{
    std::string trigItem;
    std::string title;
    long unsigned int multiplicity{};
    int rpcThr{};
    int tgcThr{};
    int sys{};//system: 1 for barrel, 2 for endcap, 3 for forward
    int threshold{};
    int charge{};
    bool tgcF{}; // full-station flag
    bool tgcC{}; // inner-coincidence flag
    bool tgcH{}; // hot roi mask flag
    bool rpcR{}; // masking feet trigger
    bool rpcM{}; // isMoreCand
    double eta{};
    double phi{};
    unsigned int roiWord{};
  };

  double getNswRindexFromEta(const double& eta) const;
  
 private:
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  SG::ReadHandleKey<TrigConf::L1Menu> m_L1MenuKey {this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","L1 Menu key"};
  SG::ReadDecorHandleKey<xAOD::MuonRoIContainer> m_thresholdPatternsKey{this,"MuRoIThresholdPatternsKey","LVL1MuonRoIs.thresholdPatterns","Name of the muon RoI container decoration for the threshold patterns"};
  BooleanProperty m_monitorThresholdPatterns{this,"MonitorThresholdPatterns",true,"start monitoring tirgger threshold patterns"};
  StringProperty m_thrPatternList{this,"ThrPatternList","MU4,MU6,MU10,MU11,MU20,MU21","list of single L1MU items to be monitored by the threshold pattern"};
  StringProperty m_streamerFilter{this,"StreamerFilter","","Streamer filter such as HLT_noalg_MU14FCH"};

  SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainerKey{this,"MuonContainerName","Muons","Offline muon track container"};
  SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerKey{this,"MuonRoIContainerName","LVL1MuonRoIs","L1 muon RoI container"};
  SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerBCm2Key{this,"MuonRoIContainerBCm2Name","LVL1MuonRoIsBCm2","L1 muon RoI container at -2BC"};
  SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerBCm1Key{this,"MuonRoIContainerBCm1Name","LVL1MuonRoIsBCm1","L1 muon RoI container at -1BC"};
  SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerBCp1Key{this,"MuonRoIContainerBCp1Name","LVL1MuonRoIsBCp1","L1 muon RoI container at +1BC"};
  SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerBCp2Key{this,"MuonRoIContainerBCp2Name","LVL1MuonRoIsBCp2","L1 muon RoI container at +2BC"};
  
  SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_TgcPrepDataContainerKey{this,"TgcPrepDataContainerName","TGC_MeasurementsAllBCs","current BC TGC PRD"};
  SG::ReadHandleKey<Muon::TgcCoinDataContainer> m_TgcCoinDataContainerPrevBCKey{this,"TgcCoinDataContainerPrevBCName","TrigT1CoinDataCollectionPriorBC","TGC Coin Data Container PrevBC"};
  SG::ReadHandleKey<Muon::TgcCoinDataContainer> m_TgcCoinDataContainerCurrBCKey{this,"TgcCoinDataContainerCurrBCName","TrigT1CoinDataCollection","TGC Coin Data Container CurrBC"};
  SG::ReadHandleKey<Muon::TgcCoinDataContainer> m_TgcCoinDataContainerNextBCKey{this,"TgcCoinDataContainerNextBCName","TrigT1CoinDataCollectionNextBC","TGC Coin Data Container NextBC"};
  SG::ReadHandleKey<Muon::TgcCoinDataContainer> m_TgcCoinDataContainerNextNextBCKey{this,"TgcCoinDataContainerNextNextBCName","TrigT1CoinDataCollectionNextNextBC","TGC Coin Data Container NextNextBC"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_PrimaryVertexContainerKey{this,"PrimaryVertexContainerName","PrimaryVertices","Primary Vertex Container"};
  
  DoubleProperty m_muonToPVdz{this,"MuonToPVdz",50.,"Window size in deltaZ between muon track and primary vertex"};
  DoubleProperty m_muonToPVdzOffset{this,"MuonToPVdzOffset",0.,"Offset of deltaZ between muon track and primary vertex"};
  DoubleProperty m_muonToPVdca{this,"MuonToPVdca",0.5,"Distance to closest approach of muon track to primary vertex"};
  BooleanProperty m_doExpressProcessing{this,"doExpressProcessing",false,"Processing express_express data"};
  StringProperty m_packageName{this,"PackageName","TgcRawDataMonitor","group name for histograming"};
  StringProperty m_ctpDecMonList{this,"CtpDecisionMoniorList","Tit:L1_2MU4,Mul:2,HLT:HLT_2mu4,RPC:1,TGC:1;","list of L1MU items to be monitored for before/after CTP decision"};
  BooleanProperty m_monitorTriggerMultiplicity{this,"MonitorTriggerMultiplicity",false,"start monitoring tirgger multiplicity performance"};
  BooleanProperty m_printAvailableMuonTriggers{this,"PrintAvailableMuonTriggers",false,"debugging purpose. print out all available muon triggers in the event"};
  BooleanProperty m_TagAndProbe{this,"TagAndProbe",true,"switch to perform tag-and-probe method"};
  BooleanProperty m_TagAndProbeZmumu{this,"TagAndProbeZmumu",false,"switch to perform tag-and-probe method Z->mumu"};
  BooleanProperty m_tagMuonInDifferentSystem{this,"TagMuonInDifferentSystem",false,"restrict the tag muons to be only in the other system (barrel or endcap)"};
  BooleanProperty m_anaTgcPrd{this,"AnaTgcPrd",false,"switch to perform analysis on TGC PRD"};
  BooleanProperty m_anaTgcCoin{this,"AnaTgcCoin",false,"switch to perform analysis on TGC Coin"};
  BooleanProperty m_fillGapByGapHistograms{this,"FillGapByGapHistograms",true,"filling gap-by-gap histograms (many many)"};
  BooleanProperty m_anaOfflMuon{this,"AnaOfflMuon",true,"switch to perform analysis on xAOD::Muon"};
  BooleanProperty m_offlMuonCutOnMuonType{this,"OfflMuonCutOnMuonType",true,"applying cut on offline muon muonType"};
  BooleanProperty m_offlMuonCutOnAuthor{this,"OfflMuonCutOnAuthor",true,"applying cut on offline muon author"};
  BooleanProperty m_offlMuonCutOnQuality{this,"OfflMuonCutOnQuality",true,"applying cut on offline muon quality"};
  BooleanProperty m_anaMuonRoI{this,"AnaMuonRoI",true,"switch to perform analysis on xAOD::LVL1MuonRoI"};
  DoubleProperty m_trigMatchWindow{this,"TrigMatchingWindow",0.2,"Window size in R for trigger matching"};
  DoubleProperty m_l1trigMatchWindowPt15{this,"L1TrigMatchingWindowPt15",0.15,"Window size in R for L1 trigger matching for 15GeV muons"};
  DoubleProperty m_l1trigMatchWindowPt10a{this,"L1TrigMatchingWindowPt10a",0.3,"Window size in R for L1 trigger matching for 10GeV(a) muons"};
  DoubleProperty m_l1trigMatchWindowPt10b{this,"L1TrigMatchingWindowPt10b",-0.01,"Window size in R for L1 trigger matching for 10GeV(b) muons"};
  DoubleProperty m_l1trigMatchWindowPt0a{this,"L1TrigMatchingWindowPt0a",0.36,"Window size in R for L1 trigger matching for 0GeV(a) muons"};
  DoubleProperty m_l1trigMatchWindowPt0b{this,"L1TrigMatchingWindowPt0b",-0.0016,"Window size in R for L1 trigger matching for 0GeV(b) muons"};
  DoubleProperty m_isolationWindow{this,"IsolationWindow",1.0,"Window size in R for isolation with other muons"};
  BooleanProperty m_requireIsolated{this,"RequireIsolated",true,"Probe muon should be isolated from other muons"};
  BooleanProperty m_useIDTrackForExtrapolation{this,"UseIDTrackForExtrapolation",false,"Use InnerDetectorTrackParticle for extrapolation"};
  BooleanProperty m_useMSTrackForExtrapolation{this,"UseMSTrackForExtrapolation",false,"Use MuonSpectrometerTrackParticle for extrapolation"};
  BooleanProperty m_useCBTrackForExtrapolation{this,"UseCBTrackForExtrapolation",false,"CombinedTrackParticle for extrapolation"};
  BooleanProperty m_useExtMSTrackForExtrapolation{this,"UseExtMSTrackForExtrapolation",false,"Use ExtrapolatedMuonSpectrometerTrackParticle for extrapolation"};
  BooleanProperty m_useMSOnlyExtMSTrackForExtrapolation{this,"UseMSOnlyExtMSTrackForExtrapolation",false,"Use MSOnlyExtrapolatedMuonSpectrometerTrackParticle for extrapolation"};
  BooleanProperty m_useDirectPrimaryTrackForExtrapolation{this,"UseDirectPrimaryTrackForExtrapolation",true,"Use DirectPrimaryTrackForExtrapolation for extrapolation"};
  BooleanProperty m_useOnlyCombinedMuons{this,"UseOnlyCombinedMuons",false,"use only CombinedMuons"};
  BooleanProperty m_useOnlyMuidCoStacoMuons{this,"UseOnlyMuidCoStacoMuons",false,"use only MuidCo and Staco Muons"};
  BooleanProperty m_useMuonSelectorTool{this,"UseMuonSelectorTool",true,"use MuonSelectorTool"};
  DoubleProperty m_pTCutOnExtrapolation{this,"pTCutOnExtrapolation",5000.,"pT [in MeV] cut on the extrapolation tracks"};
  DoubleProperty m_pTCutOnTrigExtrapolation{this,"pTCutOnTrigExtrapolation",20000.,"pT [in MeV] cut on the extrapolation tracks for trigger validation"};
  DoubleProperty m_M1_Z{this,"M1_Z",13436.5,"z-position of TGC M1-station in mm for track extrapolate"}; 
  DoubleProperty m_M2_Z{this,"M2_Z",14728.2,"z-position of TGC M2-station in mm for track extrapolate"};
  DoubleProperty m_M3_Z{this,"M3_Z",15148.2,"z-position of TGC M3-station in mm for track extrapolate"};
  DoubleProperty m_EI_Z{this,"EI_Z",7364.7,"z-position of TGC EI-station in mm for track extrapolate"};
  DoubleProperty m_FI_Z{this,"FI_Z",6978.2,"z-position of TGC FI-station in mm for track extrapolate"};
  DoubleProperty m_muonMass{this,"MuonMass",ParticleConstants::muonMassInMeV,"muon invariant mass in MeV"};
  DoubleProperty m_zMass{this,"ZMass",91187.6,"muon invariant mass in MeV"};
  DoubleProperty m_zMassWindow{this,"ZMassWindow",10000,"muon invariant mass half-window in MeV"};
  DoubleProperty m_endcapPivotPlaneMinimumRadius{this,"endcapPivotPlaneMinimumRadius",0.,"minimum radius of pivot plane in endcap region"};
  DoubleProperty m_endcapPivotPlaneMaximumRadius{this,"endcapPivotPlaneMaximumRadius", 11977.,"maximum radius of pivot plane in endcap region"};
  DoubleProperty m_barrelPivotPlaneHalfLength{this,"barrelPivotPlaneHalfLength", 9500.,"half length of pivot plane in barrel region"};
  DoubleProperty m_residualWindow{this,"ResidualWindow", 200.,"Window size in mm between hit position and track-extrapolated position"};
  DoubleProperty m_dPhiCutOnM3{this,"dPhiCutOnM3", 0.2,"Window size in delta phi on M3 between hit position and track-extrapolated position"};
  DoubleProperty m_dRCutOnM3{this,"dRCutOnM3", 3000.,"Window size in delta R (radious) on M3 between hit position and track-extrapolated position"};
  IntegerProperty m_nHitsInOtherBWTGCWire{this,"nHitsInOtherTGCWire",3,"Number of hits in other BW-TGC wire channels"};
  IntegerProperty m_nHitsInOtherBWTGCStrip{this,"nHitsInOtherTGCStrip",2,"Number of hits in other BW-TGC strip channels"};
  BooleanProperty m_dumpFullChannelList{this,"DumpFullChannelList",false,"Dump full channel list"};
  StringProperty m_maskChannelFileName{this,"MaskChannelFileName","","Name of file for mask channels"};
  DoubleProperty m_NswDeltaRCut{this,"NswDeltaRCut", 20.,"Window size in delta R for NSW-TGC matching"};

  std::vector<double> m_extZposition;
  std::vector<CtpDecMonObj> m_CtpDecMonObj;
  std::set<std::string> m_thrMonList;
  std::set<std::string> m_maskChannelList;

  using MonVariables=std::vector < std::reference_wrapper < Monitored::IMonitoredVariable >>;
  void fillTgcCoin(const std::string&,
		   const std::vector<TgcTrig>&, 
		   std::vector<Monitored::ObjectsCollection<std::vector<TgcTrig>, double>>&,
		   MonVariables&) const;
  void fillTgcCoinEff(const std::string&,
		      const std::vector<TgcTrig>&, 
		      const std::vector<ExtPos>&,
		      std::vector<ExtTrigInfo>&,
		      std::vector<Monitored::ObjectsCollection<std::vector<ExtTrigInfo>, double>>&,
		      MonVariables&) const;
  double getMatchingWindow(const xAOD::Muon* muon) const;
  
  ToolHandle<IGoodRunsListSelectorTool> m_GoodRunsListSelectorTool{this,"GRLTool","GoodRunsListSelectorTool","GoodRunsListSelectorTool"};
  ToolHandle<Trk::IExtrapolator> m_extrapolator{this,"TrackExtrapolator","Trk::Extrapolator/AtlasExtrapolator","Track extrapolator"};
  ToolHandle<ITgcRawDataMonitorTool> m_tgcMonTool{this,"TgcRawDataMonitorTool","TgcDawDataMonitorTool","TgcRawDataMonitorTool"};
  ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool{this,"MuonSelectionTool","CP::MuonSelectionTool/MuonSelectionTool","MuonSelectionTool"};
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey {this, "DetectorManagerKey","MuonDetectorManager","Key of input MuonDetectorManager condition data"}; 
  StatusCode printOutAvailableMuonTriggers() const;
  std::set<std::string> checkTriggerInfo() const;
  const xAOD::Vertex* getPrimaryVertex( const EventContext& ctx) const ;
  std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI> getRegionsOfInterest( const EventContext& ctx) const;
  void fillRoiHistograms(const std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI> & roiVec, const EventContext& ctx) const;
  void fillHistogramsAfterTriggerDecision(std::vector<TgcRawDataMonitorAlgorithm::TimedMuonRoI> & roiVec) const;
  void fillThresholdPatternHistograms(std::map<std::string,std::vector<const xAOD::MuonRoI*>> & menuAndRoIs, const std::vector < TimedMuon > & mymuons, const EventContext& ctx) const;
  void fillTgcPrdHistograms(Monitored::Scalar<int> & mon_bcid, Monitored::Scalar<int> & mon_pileup, Monitored::Scalar<int> & mon_lb, std::map < std::string, std::vector< ExtPos > > & extpositions, const EventContext & ctx) const;
  void fillMuonRoisInThresholdPattern(std::map<const xAOD::MuonRoI*,std::set<std::string>> &, std::map<std::string,std::vector<const xAOD::MuonRoI*>> &, std::vector<TimedMuonRoI> & AllBCMuonRoIs, const EventContext & ctx) const;
};
#endif
