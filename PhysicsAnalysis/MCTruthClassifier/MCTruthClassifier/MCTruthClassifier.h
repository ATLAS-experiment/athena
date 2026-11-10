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
#include "TruthUtils/TruthClasses.h"
// EDM includes
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
// For making PID selections easier
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/DecayProducts.h"

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
    // Properties used by egammaClusMatch (calorimeter cluster -> truth particle matching).
    // Angular sizes are in eta/phi units, transverse momentum cuts in GeV.
    declareProperty("FwdElectronUseG4Sel", m_FwdElectronUseG4Sel = true,
                    "Use Geant4 selection for forward electrons calo clusters. "
                    "Geant4 particles are always skipped for non-forward candidates; "
                    "if true they are skipped for forward electrons as well, if false they are considered for them");
    declareProperty("FwdElectronTruthExtrEtaCut",
                    m_FwdElectronTruthExtrEtaCut = 2.4,
                    "Cut on the eta of the truth Particles to be extrapolated "
                    "for Fwd electrons: particles with |eta| below it are not extrapolated");
    declareProperty(
        "FwdElectronTruthExtrEtaWindowCut",
        m_FwdElectronTruthExtrEtaWindowCut = 0.15,
        "Cut on the delta eta of the truth Particles to be extrapolated for "
        "Fwd electrons and the current FwdElectron: particles further away in eta are not extrapolated. "
        "Truth particles in the opposite eta hemisphere are never extrapolated");
    declareProperty("partExtrConePhi", m_partExtrConePhi = 0.4,
                    "Half-width in phi of the elliptical cone around the cluster in which truth particles are "
                    "extrapolated to the calorimeter (used if ROICone is true, and for the Geant4 particles of inclG4part)");
    declareProperty("partExtrConeEta", m_partExtrConeEta = 0.2,
                    "Half-width in eta of the elliptical cone around the cluster in which truth particles are "
                    "extrapolated to the calorimeter (used if ROICone is true, and for the Geant4 particles of inclG4part)");
    declareProperty("phtClasConePhi", m_phtClasConePhi = 0.05,
                    "Half-width in phi of the narrow elliptical cone around the cluster, "
                    "applied to the extrapolated truth particle positions; not used for forward electrons");
    declareProperty("phtClasConeEta", m_phtClasConeEta = 0.025,
                    "Half-width in eta of the narrow elliptical cone around the cluster, "
                    "applied to the extrapolated truth particle positions; not used for forward electrons");
    declareProperty("phtdRtoTrCut", m_phtdRtoTrCut = 0.1,
                    "Maximum distance dR between the cluster and the extrapolated truth particle for "
                    "a match (not forward electrons)");
    declareProperty("fwrdEledRtoTrCut", m_fwrdEledRtoTrCut = 0.15,
                    "Maximum distance dR between the cluster and the extrapolated truth particle for "
                    "a match (forward electrons)");
    declareProperty("ROICone", m_ROICone = false,
                    "Before the extrapolation, keep only truth particles inside the elliptical cone "
                    "partExtrConePhi x partExtrConeEta around the cluster (not forward electrons). "
                    "The truth particles are always taken from a fixed dR = 0.5 cone around the cluster");
    //AV: those below are needed in egammaClusMatch
    declareProperty("pTChargePartCut", m_pTChargePartCut = 1.0,
                    "Minimum pT in GeV of the charged truth particles considered for the cluster matching");
    declareProperty("pTNeutralPartCut", m_pTNeutralPartCut = 0.,
                    "Minimum pT in GeV of the neutral truth particles considered for the cluster matching");
    declareProperty("inclG4part", m_inclG4part = false,
                    "If no generator particle was matched to the cluster (not forward electrons), "
                    "also try stable Geant4 particles without a decay vertex");
#endif
#ifndef GENERATIONBASE
    // Properties used to match a track to a truth particle (getGenPart).
    declareProperty("deltaRMatchCut", m_deltaRMatchCut = 0.2,
                    "Maximum dR between a track and its truth particle for tracks with more than "
                    "NumOfSiHitsCut silicon (pixel + SCT) hits; otherwise the match is dropped");
    declareProperty("deltaPhiMatchCut", m_deltaPhiMatchCut = 0.2,
                    "Maximum delta phi between a track and its truth particle for tracks with at most "
                    "NumOfSiHitsCut silicon (pixel + SCT) hits; otherwise the match is dropped");
    declareProperty("NumOfSiHitsCut", m_NumOfSiHitsCut = 3,
                    "Number of silicon (pixel + SCT) hits on a track that decides whether deltaRMatchCut "
                    "(more hits than this) or deltaPhiMatchCut (this many or fewer) is applied");
    declareProperty("jetPartDRMatch", m_jetPartDRMatch = 0.4);
#endif
  }
  virtual ~MCTruthClassifier()  = default ;

    // Gaudi algorithm hooks
  virtual StatusCode initialize() override {
    ATH_MSG_INFO(" Initializing MCTruthClassifier");
#ifndef XAOD_ANALYSIS
    // Only needed for GenParticle interface
    ATH_CHECK(m_truthLinkVecReadHandleKey.initialize(!m_truthLinkVecReadHandleKey.key().empty()));
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

using IMCTruthClassifier::particleTruthClassifier;
  
protected:
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TruthParticle*,IMCTruthClassifier::Info* info) const override final;

public:
#ifndef GENERATIONBASE /*These can not run in Generation only release*/
  //Main method for Track to Truth association
  const xAOD::TruthParticle* getGenPart(const xAOD::TrackParticle*, IMCTruthClassifier::Info* info = nullptr) const;
#ifndef XAOD_ANALYSIS
  //Main method for egamma clusters to Truth Particle association
  const xAOD::TruthParticle* egammaClusMatch(const xAOD::CaloCluster*,bool,IMCTruthClassifier::Info* info) const;
#endif

protected:
  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::TrackParticle*,IMCTruthClassifier::Info* info) const override final;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Electron*,IMCTruthClassifier::Info* info) const override final;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Photon*,IMCTruthClassifier::Info* info) const override final;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::Muon*,IMCTruthClassifier::Info* info) const override final;

  virtual std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin>
  particleTruthClassifier(const xAOD::CaloCluster*,IMCTruthClassifier::Info* info) const override final;

#endif

private:

  MCTruthPartClassifier::ParticleOrigin defOrigOfElectron(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                          const xAOD::TruthParticle*,
                                                          bool& isPrompt,
                                                          IMCTruthClassifier::Info& info) const;

  MCTruthPartClassifier::ParticleOrigin defOrigOfMuon(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                      const xAOD::TruthParticle*,
                                                      bool& isPrompt,
                                                      IMCTruthClassifier::Info& info) const;

  MCTruthPartClassifier::ParticleOrigin defOrigOfTau(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                     const xAOD::TruthParticle*,
                                                     int motherPDG,
                                                     IMCTruthClassifier::Info& info) const;

  MCTruthPartClassifier::ParticleOrigin defOrigOfPhoton(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                        const xAOD::TruthParticle*,
                                                        bool& isPrompt,
                                                        IMCTruthClassifier::Info& info) const;

  MCTruthPartClassifier::ParticleOrigin defOrigOfNeutrino(const xAOD::TruthParticleContainer& xTruthParticleContainer,
                                                          const xAOD::TruthParticle*,
                                                          bool& isPrompt,
                                                          IMCTruthClassifier::Info& info) const;

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  bool genPartToCalo(const EventContext& ctx,
                     const xAOD::CaloCluster* clus,
                     const xAOD::TruthParticle* thePart,
                     bool isFwrdEle,
                     double& dRmatch,
                     bool& isNarrowCone,
                     const CaloDetDescrManager& caloDDMgr) const;
#endif

  /* Data members */

  /// Key of the xAOD truth particle container that is searched when classifying (ancestors, decay products, ...).
  SG::ReadHandleKey<xAOD::TruthParticleContainer>
  m_truthParticleContainerKey{this,"xAODTruthParticleContainerName","TruthParticles","ReadHandleKey for xAOD::TruthParticleContainer"};

#if !defined(XAOD_ANALYSIS) && !defined(GENERATIONBASE)
  /// Extrapolates truth particles to the calorimeter (egammaClusMatch); optional, disabled if empty.
  ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtensionTool{this,"ParticleCaloExtensionTool",""};
  /// Calorimeter detector description, needed together with the extension tool; optional, may be empty.
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager",""};
  /// Finds the truth particles in a cone around a cluster (egammaClusMatch); optional, disabled if empty.
  ToolHandle<xAOD::ITruthParticlesInConeTool>
  m_truthInConeTool{this,"TruthInConeTool","xAOD::TruthParticlesInConeTool/TruthParticlesInConeTool"};

  // Properties for the calorimeter cluster -> truth particle matching (egammaClusMatch).
  // Angular sizes are in eta/phi units, pT cuts in GeV; see the declareProperty descriptions for details.
  bool  m_FwdElectronUseG4Sel;            ///< skip Geant4 particles also for forward electrons
  float m_FwdElectronTruthExtrEtaCut;     ///< forward electrons: truth particles with |eta| below this are not extrapolated
  float m_FwdElectronTruthExtrEtaWindowCut; ///< forward electrons: max |delta eta| to the cluster for a truth particle to be extrapolated
  float m_partExtrConeEta;                ///< eta half-width of the cone in which truth particles are extrapolated
  float m_partExtrConePhi;                ///< phi half-width of the cone in which truth particles are extrapolated
  float m_phtClasConePhi;                 ///< phi half-width of the narrow cone around the cluster (not forward electrons)
  float m_phtClasConeEta;                 ///< eta half-width of the narrow cone around the cluster (not forward electrons)
  float m_phtdRtoTrCut;                   ///< max dR cluster - extrapolated truth particle (not forward electrons)
  float m_fwrdEledRtoTrCut;               ///< max dR cluster - extrapolated truth particle (forward electrons)
  bool  m_ROICone;                        ///< only extrapolate truth particles inside the partExtrCone ellipse

  float m_pTChargePartCut;                ///< minimum pT of charged truth particles considered
  float m_pTNeutralPartCut;               ///< minimum pT of neutral truth particles considered
  bool  m_inclG4part;                     ///< also try Geant4 particles if no generator particle matched
#endif

#ifndef XAOD_ANALYSIS
  SG::ReadHandleKey<xAODTruthParticleLinkVector>
  m_truthLinkVecReadHandleKey{this,"xAODTruthLinkVector","xAODTruthLinks", "ReadHandleKey for xAODTruthParticleLinkVector"};
#endif
#ifndef GENERATIONBASE
  // Properties for the track -> truth particle matching (getGenPart).
  float m_deltaRMatchCut;                 ///< max dR track - truth particle if the track has more than NumOfSiHitsCut Si hits
  float m_deltaPhiMatchCut;               ///< max delta phi track - truth particle if the track has at most NumOfSiHitsCut Si hits
  int   m_NumOfSiHitsCut;                 ///< Si (pixel + SCT) hit count selecting between deltaRMatchCut and deltaPhiMatchCut
  float m_jetPartDRMatch;
#endif
};
#endif // MCTRUTHCLASSIFIER_MCTRUTHCLASSIFIER_H
