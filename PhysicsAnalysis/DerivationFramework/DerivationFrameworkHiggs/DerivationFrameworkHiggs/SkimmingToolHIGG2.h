/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SkimmingToolHIGG2.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_SKIMMINGTOOLHSG2_H
#define DERIVATIONFRAMEWORK_SKIMMINGTOOLHSG2_H 1

#include<string>
#include<vector>
#include<map>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

// xAOD header files
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

namespace DerivationFramework {

  /** @class SkimmingToolHIGG2
      @author Susumu.Oda@cern.ch
     */
  class SkimmingToolHIGG2 : public extends<AthAlgTool, ISkimmingTool> {
    
  public: 
    /** Constructor with parameters */
    SkimmingToolHIGG2( const std::string& t, const std::string& n, const IInterface* p );
    
    /** Destructor */
    ~SkimmingToolHIGG2();
    
    // Athena algtool's Hooks
    virtual StatusCode  initialize() override;
    virtual StatusCode  finalize() override;
    
    /** Check that the current event passes this filter */
    virtual bool eventPassesFilter() const override;
    
  private:
    enum {
      NUMBER_OF_MERGED_JET_TYPES = 3
    };

    ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool;

    mutable std::atomic<unsigned int> m_ntot;
    mutable std::atomic<unsigned int> m_npass;

    bool m_skipTriggerRequirement;

    struct Candidates {
      std::vector<const xAOD::Electron*> goodElectrons;
      std::vector<const xAOD::Muon*> goodMuons;
      std::vector<const xAOD::Jet*> goodJets;
      std::vector<const xAOD::Jet*> goodMergedJets[NUMBER_OF_MERGED_JET_TYPES];
      std::vector<const xAOD::Photon*> goodPhotons;
      std::vector<const xAOD::TrackParticle*> goodTracks;
    };

    std::string m_filterType;

    std::string m_electronSGKey;
    std::string m_muonSGKey;
    std::string m_jetSGKey;
    std::string m_mergedJetSGKey[NUMBER_OF_MERGED_JET_TYPES];
    std::string m_photonSGKey;
    std::string m_trackSGKey;

    unsigned int m_nLeptons;
    unsigned int m_nElectrons;
    unsigned int m_nMuons;
    unsigned int m_nJets;
    unsigned int m_nMergedJets[NUMBER_OF_MERGED_JET_TYPES]{};
    unsigned int m_nPhotons;
    unsigned int m_nTracks;

    std::string m_electronQual;
    std::string m_muonQual;
    std::string m_tightMuonQual;
    std::string m_tightElectronQual;
    std::string m_jetQual;
    std::string m_mergedJetQual[NUMBER_OF_MERGED_JET_TYPES];
    std::string m_photonQual;

    std::string m_primaryElectronQual4L;

    std::vector<std::string> m_trigger2L;
    std::vector<std::string> m_triggerTP;
    std::vector<std::string> m_trigger2L2Q;
    std::vector<std::string> m_triggerJPSI;
    std::vector<std::string> m_triggerPHI;

    double m_electronEtCut;
    double m_muonPtCut;
    double m_tightElectronEtCut;
    double m_tightMuonPtCut;
    double m_jetPtCut;
    double m_mergedJetPtCut[NUMBER_OF_MERGED_JET_TYPES]{};
    double m_photonPtCut;
    double m_trackPtCut;

    double m_electronEtaCut;
    double m_muonEtaCut;
    double m_caloMuonEtaCut;
    double m_jetEtaCut;
    double m_mergedJetEtaCut[NUMBER_OF_MERGED_JET_TYPES]{};
    double m_photonEtaCut;

    double m_invariantMassCut;
    double m_invariantMassJpsiLowCut;
    double m_invariantMassJpsiUpCut;
    double m_invariantMassUpsilonLowCut;
    double m_invariantMassUpsilonUpCut;
    double m_invariantMassPhiLowCut;
    double m_invariantMassPhiUpCut;

    double m_leadingElectronEtCut;
    double m_leadingMuonPtCut;

    double m_dRElectronJetCut;

    bool m_checkLArError;
    bool m_defaultElectronFourMomentum; 

    //    bool m_DFCommonElectronFourMomentum; 
    //    mutable std::map<const xAOD::Electron*, unsigned int> m_mapElectrons;
    //    mutable const std::vector<float> *m_DFCommonElectrons_pt;
    //    mutable const std::vector<float> *m_DFCommonElectrons_eta;
    //    mutable const std::vector<float> *m_DFCommonElectrons_phi;

    bool m_DFCommonJetFourMomentum; 

    bool m_requireTightLeptons;

    bool checkElectronQuality(const xAOD::Electron *el, const bool isTight=false) const;
    bool checkMuonQuality(const xAOD::Muon *mu, const bool isTight=false) const;
    bool checkJetQuality(const xAOD::Jet *jet) const;
    bool checkMergedJetQuality(const xAOD::Jet *jet, const unsigned int type) const;
    bool checkPhotonQuality(const xAOD::Photon *ph) const;
    bool checkTrackQuality(const xAOD::TrackParticle *trk) const;

    bool check2L(const Candidates& evt) const;
    bool check4L(const Candidates& evt) const;
    bool checkTP(const Candidates& evt) const;
    bool check2L2Q(const Candidates& evt) const;
    bool checkJPSI(const Candidates& evt) const;
    bool checkPHI(const Candidates& evt) const;

    TLorentzVector electronFourMomentum(const xAOD::Electron *el) const;
    static TLorentzVector muonFourMomentum(const xAOD::Muon *mu) ;
    TLorentzVector jetFourMomentum(const xAOD::Jet *jet) const;

    static double getDeltaR(const double eta1, const double phi1, const double eta2, const double phi2) ; 

    static const double s_MZ;
    static const double s_MKplus;
  }; 
  
}

#endif // DERIVATIONFRAMEWORK_SKIMMINGTOOLEXAMPLE_H
