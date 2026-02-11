#ifndef IPNTUPLEDUMPER_H
#define IPNTUPLEDUMPER_H

#include <AnaAlgorithm/AnaAlgorithm.h>

// c++ include(s):
#include <iterator>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "AthLinks/ElementLink.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"

#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "TrackVertexAssociationTool/TrackVertexAssociationTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackBiasingTool.h"

#include "IPPerformance/ETAlgorithm.h"
#include "IPPerformance/TrackTruthHelper.h"
#include "IPPerformance/ReturnCheck.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#ifndef __MAKECINT__
#include "xAODTruth/TruthParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"
#endif // not __MAKECINT__

//xAOD
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/Vertex.h"

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// IP studies
#include "IPPerformance/IPhistos.h"

class IPNtupleDumper : public ETAlgorithm
{

private:
  ToolHandle<TrigConf::xAODConfigTool>      m_trigConfTool{this,"trigConfTool","TrigConf::xAODConfigTool"};
  ToolHandle<Trig::TrigDecisionTool>        m_trigDecTool{this, "trigDecTool", "Trig::TrigDecisionTool/TrigDecisionTool"};
  SG::WriteDecorHandleKey<xAOD::EventInfo>  m_trigDecKey {this, "trigDecKey", "EventInfo.passTrigDecision", "Decoration for Trigger Decision"};

  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  //Config variables
  std::string m_inRecTrkContainer;                //! input track container name
  std::string m_vtxContainer;                     //! vtx container name
  std::string m_inJetContainer;                   //! input jet container name
  std::string m_histOutput;                       //! output file name
  std::string derivationName;                   //! derivation name for the IP decorations
  float       m_TruthPtCut;                       //! Limit to do track-jet association
  float       m_TruthEtaCut;                      //! Limit to do track-jet association
  float       m_TruthMatchProb;                   //! Limit to do track-jet association
  bool        m_doTightTruthMatch;                //! Store info about matching truth particles
  bool        m_doGhostAssociation;               //! RetrieveTracks via ghostAssociation (true) or deltaR matching (false) [not implemented yet]
  float       m_deltaRCut;                        //! Limit to do track-jet association
  bool        m_applyDeltaRCut;                   //! Cut on tracks that have deltaR(trk,jet) > m_deltaRCut
  bool m_skipPVfailures;                          //! skip the tracks that have 999 in the IPsigma struct
  std::string m_TTVA_WP;                          //! TTVA working point
  bool m_noTTVAonInput;                           //! Option to run on samples without correct TTVA variables (i.e. all before reprocessing)
  bool m_debug;                                   //! debug
  
  Gaudi::Property<bool> ipHLTcorrection{this,"ipHLTcorrection",false,"Flags to HLTcorrection"}; // Passed as a flag in the runAnalysis command line. Include or not HLT prescale correction in data weights.

  // Flags and variables used for IP studies 
  Gaudi::Property<bool> ipSaveHistosOnly{this, "ipSaveHistosOnly", false, "Flag to save histograms only"};   // Passed as a flag in the runAnalysis command line. Save the IP histograms only without dumping an IP ntuple.

  Gaudi::Property<bool> ipSaveAdditionalHistos{this,"ipSaveAdditionalHistos",false,"Flag to save AdditionalHistos"};
  std::unique_ptr<IPhistos> m_IPhistos;           //!
  Gaudi::Property<std::string> m_configFileName{this, "configFileName", "", "config file name"};
  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)

public:

  int m_eventCounter;     //!
  Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};
  TrackTruthHelpers* m_truthhelper; //!
  std::vector<InDet::InDetTrackSelectionTool*> m_trackselectionTools; //!
  ToolHandle<InDet::InDetTrackSelectionTool>    m_LoosePrimary_selTool{this,"LoosePrimary_selTool","InDet::InDetTrackSelectionTool"};
  ToolHandle<InDet::InDetTrackSelectionTool>    m_TightPrimary_selTool{this,"TightPrimary_selTool","InDet::InDetTrackSelectionTool"};
  ToolHandle<CP::TrackVertexAssociationTool>    m_trktovxtool{this,"trktovxtool","CP::TrackVertexAssociationTool"};
  //InDet::InDetTrackBiasingTool *m_trackSmearingTool2;
  
  TTree *t1; //!
  
  TH1D* h_SumOfEventWeights = nullptr; //! //MVGR: to help getting total SumOfWeights of MC slices
  TH1D* h_jetPt; //!  //no use
  TH1D* h_jetPt_passSel; //!
  TH2D* h_rtrack; //!
  TH2D* h_ntrack; //!
  TH1D* h_pTratio; //!
  
  // this is a standard constructor
  IPNtupleDumper (const std::string& name, ISvcLocator* pSvcLocator = nullptr);
  virtual ~IPNtupleDumper();  

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize ();
  virtual StatusCode execute ();
  virtual StatusCode finalize ();

  // these are the functions not inherited from Algorithm
  
  void SetBranches(TTree* t);
  void ResetVars();
  bool CheckForAvailableDecorations(const xAOD::TrackParticleContainer* trkC, const std::string& derivationName);

  std::string GetDerivationName(const xAOD::TrackParticleContainer* trkC);
  
  void TrackToJetDeltaRAssociation(const xAOD::TrackParticle* trk, const xAOD::JetContainer* jets, float dRcut,float& deltaR, float& pT, float& Etajet, int& JVTjet);

  //Get if a jet passes the JVT
  int PassJVTCut(const xAOD::Jet* jet);

  //Using all tracks and Delta R matching
  bool ProcessTrack(const xAOD::TrackParticle* track, const xAOD::JetContainer* jets, const xAOD::VertexContainer* vtxCont);
  
  //Using ghost associated tracks
  bool ProcessTrack(const xAOD::TrackParticle* track, const xAOD::Jet* jet, const xAOD::VertexContainer* vtxCont);
  
  void FillTreeVariables(const xAOD::TrackParticle* track,float deltaR_trk_jet, float pTjet, float Etajet, int JVTjet);

  // Functions used when filling histograms for IP studies
  StatusCode FillIPHistograms();
  StatusCode GetIntLumi(Int_t runN, float& intLumi);
  std::vector<int> Classify(unsigned int i);
  
  //Get the truth link of a track
  //This should go in a helper
  //From https://gitlab.cern.ch:8443/nstyles/TruthStudies/blob/master/source/TruthAnalysis/src/TruthAnalysisAlg.cxx
  
  typedef ElementLink <xAOD::TruthParticleContainer > Link_t;
  
  const xAOD::TruthParticle* getTrackTruthLink(const xAOD::TrackParticle* track)  const ;
    
  const xAOD::TruthParticle* getAssociatedPrimaryTruth(const xAOD::TrackParticle* track)  const;
  
  //========Tree Output Branches=======
  
  int ntracks; //!
  int runN;    //!
  int evtN;    //!
  float evtW;  //!
  int lb;      //!
  float mu;    //!
  float bsx;   //!
  float bsy;   //!
  float bsz;   //!
  float bsSigmax;   //!
  float bsSigmay;   //!
  float bsSigmaz;   //!
  float pvx;   //!
  float pvy;   //!
  float pvz;   //!
  int   pvN;   //!
  
  //0 - LoosePrimary 
  //1 - TightPrimary 
  //2 - 7 not Defined yet => To Do
  std::vector<uint8_t> selectionBits; //!
  std::vector<float> d0_pv;         //!
  std::vector<float> z0_pv;         //!
  std::vector<float> d0Sigma_pv;    //!
  std::vector<float> z0Sigma_pv;    //!
  std::vector<float> d0PVSigma_pv;  //!
  std::vector<float> z0PVSigma_pv;  //!
  std::vector<float> trk_pt;        //! 
  std::vector<int>   trk_q;         //!
  std::vector<unsigned int> trk_hitPattern;//!
  std::vector<float> trk_chi2;//!
  std::vector<float> trk_eta;       //!
  std::vector<float> trk_theta;     //!
  std::vector<float> trk_phi;       //!
  std::vector<float> trk_d0;       //!
  std::vector<float> trk_z0;       //!
  std::vector<float> trk_d0Sigma;    //!
  std::vector<float> trk_z0Sigma;    //!
  std::vector<float> deltaRtrkJet;  //!
  std::vector<uint8_t> trk_nInnermostPixelLayerHits; //!
  std::vector<uint8_t> trk_nInnermostPixelLayerOutliers; //!
  std::vector<uint8_t> trk_nInnermostPixelLayerSharedHits; //!
  std::vector<uint8_t> trk_nInnermostPixelLayerSplitHits; //!
  std::vector<uint8_t> trk_expectInnermostPixelLayerHit; //!
  std::vector<uint8_t> trk_nNextToInnermostPixelLayerHits; //!
  std::vector<uint8_t> trk_nNextToInnermostPixelLayerOutliers; //!
  std::vector<uint8_t> trk_nNextToInnermostPixelLayerSharedHits; //!
  std::vector<uint8_t> trk_nNextToInnermostPixelLayerSplitHits; //!
  std::vector<uint8_t> trk_expectNextToInnermostPixelLayerHit; //!
  std::vector<uint8_t> trk_nPixelSharedHits; //!
  std::vector<uint8_t> trk_nPixelSplitHits; //!
  std::vector<uint8_t> trk_nSCTSharedHits; //!
  std::vector<float> jetPt;         //!
  std::vector<float> jetEta;         //!
  std::vector<int> jetJVT;        //! 

  // MVGR
  std::vector<float> jetPt_Uncalibrated;     
  std::vector<unsigned char> numberOfPixelHits;
  std::vector<unsigned char> numberOfPixelHoles;
  std::vector<unsigned char> numberOfPixelOutliers;
  std::vector<unsigned char> numberOfPixelSharedHits;
  std::vector<unsigned char> numberOfPixelSplitHits;
  std::vector<unsigned char> numberOfSCTHits;
  std::vector<unsigned char> numberOfSCTHoles;
  std::vector<unsigned char> numberOfSCTOutliers;
  std::vector<unsigned char> numberOfSCTSharedHits;

  std::vector<float> trk_truth_pt;  //!
  std::vector<int> trk_barcode;       //!
  std::vector<int> trk_pdgId;       //!
  std::vector<float> trk_truth_prodR; //!
  std::vector<float> trk_truth_phi; //!
  std::vector<float> trk_truth_d0; //!
  std::vector<float> trk_truth_z0; //!

  //For the systematics
  //InDet::InDetTrackTruthFilterTool *m_truthFilterTool; //!
  ToolHandle<InDet::InDetTrackTruthFilterTool>    m_truthFilterTool{this,"truthFilterTool","InDet::InDetTrackTruthFilterTool"};
  //===================================
  
  
  // this is needed to distribute the algorithm to the workers
  //ClassDef(IPNtupleDumper, 1);
};

#endif

