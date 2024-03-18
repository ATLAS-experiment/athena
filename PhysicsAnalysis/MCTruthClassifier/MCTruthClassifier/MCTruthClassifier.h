/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
#define MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
/********************************************************************
NAME:     MCTruthClassifier.h
PACKAGE:  atlasoff/PhysicsAnalysis/MCTruthClassifier
AUTHORS:  O. Fedin
CREATED:  Sep 2007
 ********************************************************************/

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/AsgTool.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
// EDM includes
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
// For making PID selections easier
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/MagicNumbers.h"

#ifndef XAOD_ANALYSIS
#include "GaudiKernel/ToolHandle.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "AtlasHepMC/GenParticle.h"
#endif

#ifndef GENERATIONBASE
//EDM includes
#include "xAODTracking/TrackParticle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#endif

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "ParticlesInConeTools/ITruthParticlesInConeTool.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "AthenaKernel/Units.h"
#endif

#include <cmath>
#include <utility>
class MCTruthClassifier : virtual public IMCTruthClassifier , public asg::AsgTool
{
  ASG_TOOL_CLASS(MCTruthClassifier, IMCTruthClassifier)
public:
  // constructor
  MCTruthClassifier(const std::string& type)  : asg::AsgTool(type) {
#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
    declareProperty("FwdElectronUseG4Sel" , m_FwdElectronUseG4Sel = true, "Use Geant4 selection for forward electrons calo clusters");
    declareProperty("FwdElectronTruthExtrEtaCut", m_FwdElectronTruthExtrEtaCut = 2.4, "Cut on the eta of the truth Particles to be extrapolated for Fwd electrons");
    declareProperty("FwdElectronTruthExtrEtaWindowCut", m_FwdElectronTruthExtrEtaWindowCut = 0.15, "Cut on the delta eta of the truth Particles to be extrapolated for Fwd electrons and the current FwdElectron");
    declareProperty("partExtrConePhi", m_partExtrConePhi = 0.4);
    declareProperty("partExtrConeEta", m_partExtrConeEta = 0.2);
    declareProperty("phtClasConePhi", m_phtClasConePhi = 0.05);
    declareProperty("phtClasConeEta", m_phtClasConeEta = 0.025);
    declareProperty("useCaching", m_useCaching = true);
    declareProperty("phtdRtoTrCut", m_phtdRtoTrCut = 0.1);
    declareProperty("fwrdEledRtoTrCut", m_fwrdEledRtoTrCut = 0.15);
    declareProperty("ROICone", m_ROICone = false);
//AV: those below are needed in egammaClusMatch
    declareProperty("pTChargePartCut", m_pTChargePartCut = 1.0);
    declareProperty("pTNeutralPartCut", m_pTNeutralPartCut = 0.);
    declareProperty("inclG4part", m_inclG4part = false);
#endif
#ifndef GENERATIONBASE
    declareProperty("deltaRMatchCut", m_deltaRMatchCut = 0.2);
    declareProperty("deltaPhiMatchCut", m_deltaPhiMatchCut = 0.2);
    declareProperty("NumOfSiHitsCut", m_NumOfSiHitsCut = 3);
    declareProperty("jetPartDRMatch", m_jetPartDRMatch = 0.4);
#endif
    // Properties Available in all
    declareProperty("inclEgammaFwrdEle", m_inclEgammaFwrdEle = true);
    declareProperty("inclEgammaPhoton", m_inclEgammaPhoton = true);
  } 
  virtual ~MCTruthClassifier()  = default ;

    // Gaudi algorithm hooks
  virtual StatusCode initialize() override {
    ATH_MSG_INFO(" Initializing MCTruthClassifier");
#ifndef XAOD_ANALYSIS
    // Only needed for GenParticle interface
    if (!m_truthLinkVecReadHandleKey.key().empty()) {
      ATH_CHECK(m_truthLinkVecReadHandleKey.initialize());
    }
#endif
    ATH_CHECK(m_truthParticleContainerKey.initialize());

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
    if (!m_caloExtensionTool.empty()) {
      ATH_CHECK(m_caloExtensionTool.retrieve());
    } else {
      m_caloExtensionTool.disable();
    }

    ATH_CHECK(m_caloMgrKey.initialize(SG::AllowEmpty));

    if (!m_truthInConeTool.empty()) {
      ATH_CHECK(m_truthInConeTool.retrieve());
    } else {
      m_truthInConeTool.disable();
    }
#endif
    return StatusCode::SUCCESS;
  }  
  virtual StatusCode finalize()
#ifndef XAOD_STANDALONE
    override
#endif
  { return StatusCode::SUCCESS;}

  /* All get to see these*/
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::TruthParticle*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  checkOrigOfBkgElec(const xAOD::TruthParticle* thePart,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual unsigned int classify(const xAOD::TruthParticle*) const override;

  virtual const xAOD::TruthParticle* getParentHadron(const xAOD::TruthParticle*) const override;

  enum MCTC_bits : unsigned int { HadTau=0, Tau, hadron, frombsm, uncat, isbsm, isgeant, stable, totalBits };

  /*! \brief This helper function returns the value -1 by checking the bit set in \ref MCTruthClassifier.
   * It returns the value -1 if uncategorised, 0 if non-prompt, 1 if prompt
   * It also checks for prompt taus
   */
  static int isPrompt(const unsigned int classify, bool allow_prompt_tau_decays = true) {
    std::bitset<MCTC_bits::totalBits> res(classify);
    if (res.test(MCTC_bits::uncat)) return -1;
    bool fromPromptTau = res.test(MCTC_bits::Tau) && !res.test(MCTC_bits::HadTau);
    if (fromPromptTau) return int(allow_prompt_tau_decays);
    return !res.test(MCTC_bits::hadron);
  }

#ifndef XAOD_ANALYSIS /*These can not run in Analysis Base*/
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const HepMcParticleLink& theLink,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(HepMC::ConstGenParticlePtr,MCTruthPartClassifier::Info* info = nullptr) const override;

#endif

#ifndef GENERATIONBASE /*These can not run in Generation only release*/
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::TrackParticle*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::Electron*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::Photon*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::Muon*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> 
  particleTruthClassifier(const xAOD::CaloCluster*,MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Jet*, bool DR, MCTruthPartClassifier::Info* info = nullptr) const override;

  virtual const xAOD::TruthParticle* getGenPart(const xAOD::TrackParticle*, MCTruthPartClassifier::Info* info = nullptr) const override;

#endif

private:
  inline double detEta(double x, double y) const { return std::abs(x - y); }
  inline double detPhi(double x, double y)  const {
    double det = x - y;
    if (det > M_PI) det = det - 2. * M_PI;
    if (det < -M_PI) det = det + 2. * M_PI;
    return std::abs(det);
  }

  static MCTruthPartClassifier::ParticleType defTypeOfElectron(MCTruthPartClassifier::ParticleOrigin, bool isPrompt) ;
  MCTruthPartClassifier::ParticleOrigin defOrigOfElectron(const xAOD::TruthParticleContainer* xTruthParticleContainer,
                                                          const xAOD::TruthParticle*,
                                                          bool& isPrompt,
                                                          MCTruthPartClassifier::Info* info) const;
  static MCTruthPartClassifier::ParticleOutCome defOutComeOfElectron(const xAOD::TruthParticle*) ;

  static MCTruthPartClassifier::ParticleType defTypeOfMuon(MCTruthPartClassifier::ParticleOrigin, bool isPrompt) ;
  MCTruthPartClassifier::ParticleOrigin defOrigOfMuon(const xAOD::TruthParticleContainer* m_xTruthParticleContainer,
                                                      const xAOD::TruthParticle*,
                                                      bool& isPrompt,
                                                      MCTruthPartClassifier::Info* info) const;
  static MCTruthPartClassifier::ParticleOutCome defOutComeOfMuon(const xAOD::TruthParticle*) ;

  static MCTruthPartClassifier::ParticleType defTypeOfTau(MCTruthPartClassifier::ParticleOrigin);
  MCTruthPartClassifier::ParticleOrigin defOrigOfTau(const xAOD::TruthParticleContainer* m_xTruthParticleContainer,
                                                     const xAOD::TruthParticle*,
                                                     int motherPDG,
                                                     MCTruthPartClassifier::Info* info) const;
  MCTruthPartClassifier::ParticleOutCome defOutComeOfTau(const xAOD::TruthParticle*, MCTruthPartClassifier::Info* info) const;

  static MCTruthPartClassifier::ParticleType defTypeOfPhoton(MCTruthPartClassifier::ParticleOrigin) ;
  MCTruthPartClassifier::ParticleOrigin defOrigOfPhoton(const xAOD::TruthParticleContainer* m_xTruthParticleContainer,
                                                        const xAOD::TruthParticle*,
                                                        bool& isPrompt,
                                                        MCTruthPartClassifier::Info* info) const;
  static MCTruthPartClassifier::ParticleOutCome defOutComeOfPhoton(const xAOD::TruthParticle*) ;

  MCTruthPartClassifier::ParticleOrigin defOrigOfNeutrino(const xAOD::TruthParticleContainer* m_xTruthParticleContainer,
                                                          const xAOD::TruthParticle*,
                                                          bool& isPrompt,
                                                          MCTruthPartClassifier::Info* info) const;
  //MCTruthPartClassifier::ParticleOrigin
  std::tuple<unsigned int, const xAOD::TruthParticle*> defOrigOfParticle(const xAOD::TruthParticle*) const;

  MCTruthPartClassifier::ParticleOrigin defHadronType(int);
  static MCTruthPartClassifier::ParticleType defTypeOfHadron(int);
  static MCTruthPartClassifier::ParticleOrigin convHadronTypeToOrig(MCTruthPartClassifier::ParticleType pType,int motherPDG);

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  bool genPartToCalo(const EventContext& ctx,
                     const xAOD::CaloCluster* clus,
                     const xAOD::TruthParticle* thePart,
                     bool isFwrdEle,
                     double& dRmatch,
                     bool& isNarrowCone,
                     const CaloDetDescrManager& caloDDMgr) const;

  const xAOD::TruthParticle* egammaClusMatch(const xAOD::CaloCluster*,bool,MCTruthPartClassifier::Info* info) const;
#endif

#ifndef GENERATIONBASE
  double fracParticleInJet(const xAOD::TruthParticle*, const xAOD::Jet*, bool DR, bool nparts) const;
  void findJetConstituents(const xAOD::Jet*, std::set<const xAOD::TruthParticle*>& constituents, bool DR) const;
#endif

  static MCTruthPartClassifier::ParticleOrigin defJetOrig(const std::set<const xAOD::TruthParticle*>&) ;

  /* Data members*/
  SG::ReadHandleKey<xAOD::TruthParticleContainer> 
  m_truthParticleContainerKey{this,"xAODTruthParticleContainerName","TruthParticles","ReadHandleKey for xAOD::TruthParticleContainer"};
  bool m_inclEgammaPhoton;
  bool m_inclEgammaFwrdEle;

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtensionTool{this,"ParticleCaloExtensionTool",""};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager",""};
  ToolHandle<xAOD::ITruthParticlesInConeTool> 
  m_truthInConeTool{this,"TruthInConeTool","xAOD::TruthParticlesInConeTool/TruthParticlesInConeTool"};

  bool  m_FwdElectronUseG4Sel;
  float m_FwdElectronTruthExtrEtaCut;
  float m_FwdElectronTruthExtrEtaWindowCut;
  float m_partExtrConeEta;
  float m_partExtrConePhi;
  bool m_useCaching;
  float m_phtClasConePhi;
  float m_phtClasConeEta;
  float m_phtdRtoTrCut;
  float m_fwrdEledRtoTrCut;
  bool m_ROICone;

  float m_pTChargePartCut;
  float m_pTNeutralPartCut;
  bool m_inclG4part;  
#endif

#ifndef XAOD_ANALYSIS
  SG::ReadHandleKey<xAODTruthParticleLinkVector> 
  m_truthLinkVecReadHandleKey{this,"xAODTruthLinkVector","xAODTruthLinks", "ReadHandleKey for xAODTruthParticleLinkVector"};
#endif
#ifndef GENERATIONBASE
  float m_deltaRMatchCut;
  float m_deltaPhiMatchCut;
  int m_NumOfSiHitsCut;
  float m_jetPartDRMatch;
#endif
};
#endif // MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
