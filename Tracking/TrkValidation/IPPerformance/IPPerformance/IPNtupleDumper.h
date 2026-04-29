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

#include "AthenaBaseComps/AthAlgorithm.h"
#include "IPPerformance/ReturnCheck.h"
#include "GaudiKernel/ToolHandle.h"
#ifndef __MAKECINT__
#include "xAODTruth/TruthParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"
#endif // not __MAKECINT__

//xAOD
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/Vertex.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

// IP studies
#include "IPPerformance/IPhistos.h"

class TruthMatchProbabilityCut {
 protected:
  double m_truthmatchprobabilitycut;
 public:
 TruthMatchProbabilityCut(double truthmatchprobabilitycut = 0.5) :
  m_truthmatchprobabilitycut (truthmatchprobabilitycut) {};
  bool accept(const xAOD::TrackParticle* track, const xAOD::Vertex*) const {
    static const SG::Accessor<float> mAcc_truthMatchProbability("truthMatchProbability");
    if( !mAcc_truthMatchProbability(*track)) {
      Warning("TruthMatchProbabilityCut()", "Track Particle has no MatchProb! Is this data?" );
      return true;
    }
    const SG::Accessor<float> mAcc_truthProb("truthMatchProbability");
    const float truthProb = mAcc_truthProb(*track);
    return ( truthProb >= m_truthmatchprobabilitycut );
  }
};

class IPNtupleDumper : public AthAlgorithm
{

private:
  PublicToolHandle<Trig::TrigDecisionTool>        m_trigDecTool{this, "trigDecTool", "Trig::TrigDecisionTool/TrigDecisionTool"};

  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  //Config variables
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackParticlesKey", "InDetTrackParticles"};
  std::string m_vtxContainer;                     //! vtx container name
  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{this, "JetsKey", "AntiKt4EMTopoJets_Selected"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_jetConstitScalePtKey{this, "JetConstitScalePt", m_jetKey, "JetConstitScaleMomentum_pt"};

  // IDTIDE
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_d0_IDTIDE_key{this, "d0IDTIDEKey", m_trackKey, "IDTIDE_unbiased_d0", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_z0_IDTIDE_key{this, "z0IDTIDEKey", m_trackKey, "IDTIDE_unbiased_z0", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_d0Sigma_IDTIDE_key{this, "d0SigmaIDTIDEKey", m_trackKey, "IDTIDE_unbiased_d0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_z0Sigma_IDTIDE_key{this, "z0SigmaIDTIDEKey", m_trackKey, "IDTIDE_unbiased_z0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_PVd0Sigma_IDTIDE_key{this, "PVd0SigmaIDTIDEKey", m_trackKey, "IDTIDE_unbiased_PVd0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_PVz0Sigma_IDTIDE_key{this, "PVz0SigmaIDTIDEKey", m_trackKey, "IDTIDE_unbiased_PVz0Sigma", ""};

  // IDTIDE1
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_d0_IDTIDE1_key{this, "d0IDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_d0", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_z0_IDTIDE1_key{this, "z0IDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_z0", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_d0Sigma_IDTIDE1_key{this, "d0SigmaIDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_d0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_z0Sigma_IDTIDE1_key{this, "z0SigmaIDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_z0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_PVd0Sigma_IDTIDE1_key{this, "PVd0SigmaIDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_PVd0Sigma", ""};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_PVz0Sigma_IDTIDE1_key{this, "PVz0SigmaIDTIDE1Key", m_trackKey, "IDTIDE1_unbiased_PVz0Sigma", ""};
  bool m_useIDTIDE = false;

  std::string derivationName;                   //! derivation name for the IP decorations
  Gaudi::Property<float> m_TruthPtCut{this, "TruthPtCut", 500, "Limit to do track-jet association"};
  Gaudi::Property<float> m_TruthEtaCut{this, "TruthEtaCut", 2.5, "Limit to do track-jet association"};
  Gaudi::Property<float> m_TruthMatchProb{this, "TruthMatchProb", 0.5, "Limit to do track-jet association"};
  Gaudi::Property<bool>  m_doTightTruthMatch{this, "DoTightTruthMatch", true, "Store info about matching truth particles"};
  Gaudi::Property<bool>  m_doGhostAssociation{this, "DoGhostAssociation", true, "RetrieveTracks via ghostAssociation (true) or deltaR matching (false)"};
  Gaudi::Property<float> m_deltaRCut{this, "DeltaRCut", 0.4, "DeltaR cut for matching"};

  Gaudi::Property<bool> m_ipHLTcorrection{this,"ipHLTcorrection",true,"Flags to HLTcorrection"}; // Passed as a flag in the runAnalysis command line. Include or not HLT prescale correction in data weights.

  // Flags and variables used for IP studies 
  Gaudi::Property<bool> m_ipSaveHistosOnly{this, "ipSaveHistosOnly", true, "Flag to save histograms only"};   // Passed as a flag in the runAnalysis command line. Save the IP histograms only without dumping an IP ntuple.

  Gaudi::Property<bool> m_ipSaveAdditionalHistos{this,"ipSaveAdditionalHistos",true,"Flag to save AdditionalHistos"};
  std::unique_ptr<IPhistos> m_IPhistos;           //!
  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)


  Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};
  //TrackTruthHelpers* m_truthhelper; //!
  ToolHandleArray<InDet::IInDetTrackSelectionTool> m_trackselectionTools{this, "trackSelectionTools", {}};
  ToolHandle<CP::TrackVertexAssociationTool>    m_trktovxtool{this,"trktovxtool","CP::TrackVertexAssociationTool"};
  
  TTree *m_t1; //!
  
  TH1D* m_h_SumOfEventWeights = nullptr; //! //MVGR: to help getting total SumOfWeights of MC slices

  public:
  
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
  bool CheckForAvailableDecorations(const xAOD::TrackParticleContainer* trkC, const std::string& m_derivationName);
  StatusCode CheckIPDecorations(const EventContext& ctx);
  const xAOD::TruthParticle* truthParticle(const xAOD::TrackParticle* ) const;
  bool passAcceptance(const xAOD::TruthParticle* truth) const;
  std::string GetDerivationName(const xAOD::TrackParticleContainer* trkC);
  
  void TrackToJetDeltaRAssociation(const xAOD::TrackParticle* trk, const xAOD::JetContainer* jets, float dRcut,float& deltaR, float& pT, float& Etajet, int& JVTjet);

  //Get if a jet passes the JVT
  bool PassJVTCut(const xAOD::Jet* jet);

  //Using all tracks and Delta R matching
  bool ProcessTrack_DeltaR(const xAOD::TrackParticle* track, const xAOD::JetContainer* jets, const xAOD::VertexContainer* vtxCont);
  
  //Using ghost associated tracks
  bool ProcessTrack_GhostAssoc(const xAOD::TrackParticle* track, const xAOD::Jet* jet, const xAOD::VertexContainer* vtxCont);
  
  void FillTreeVariables(const xAOD::TrackParticle* track,float deltaR_trk_jet, float pTjet, float Etajet, bool JVTjet);

  // Functions used when filling histograms for IP studies
  StatusCode FillIPHistograms();
  std::vector<int> Classify(unsigned int i);
  
  //Get the truth link of a track
  //This should go in a helper
  //From https://gitlab.cern.ch:8443/nstyles/TruthStudies/blob/master/source/TruthAnalysis/src/TruthAnalysisAlg.cxx
  
  typedef ElementLink <xAOD::TruthParticleContainer > Link_t;
  
  const xAOD::TruthParticle* getTrackTruthLink(const xAOD::TrackParticle* track)  const ;
    
  const xAOD::TruthParticle* getAssociatedPrimaryTruth(const xAOD::TrackParticle* track)  const;

  bool isPrimary(const xAOD::TrackParticle* track) const;
  bool isPrimaryParticle(const xAOD::TruthParticle* truth) const;
  
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
  ToolHandle<InDet::InDetTrackTruthFilterTool>    m_truthFilterTool{this,"truthFilterTool","InDet::InDetTrackTruthFilterTool"};
  //===================================
  
  
};

#endif

