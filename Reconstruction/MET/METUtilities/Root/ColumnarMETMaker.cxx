///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ColumnarMETMaker.cxx
// Implementation file for class ColumnarMETMaker
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////

// METUtilities includes
#include "METUtilities/ColumnarMETMaker.h"
#include "METUtilities/METHelpers.h"

// MET EDM
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAssociationHelper.h"

// Jet EDM
#include "xAODJet/JetAttributes.h"

// Tracking EDM
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"

// Shallow copy
#include "xAODCore/ShallowCopy.h"

// Muon EDM
#include "xAODMuon/MuonContainer.h"

// Electron EDM
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

// framework includes
#include "AsgDataHandles/ReadHandle.h"
#include <AsgTools/AsgToolConfig.h>
#include "xAODPFlow/PFOAuxContainer.h"
#include <xAODCore/AuxContainerBase.h>
#include <AthContainers/AuxElement.h>

#include <memory>

namespace met {

  using xAOD::MissingET;
  using xAOD::MissingETContainer;
  using xAOD::MissingETAssociation;
  using xAOD::MissingETAssociationMap;
  using xAOD::MissingETAuxContainer;
  using xAOD::MissingETComposition;
  using xAOD::IParticle;
  using xAOD::IParticleContainer;
  using xAOD::JetContainer;
  using xAOD::JetConstituentVector;
  using xAOD::TrackParticle;

  using iplink_t = ElementLink<xAOD::IParticleContainer>;
  static const SG::AuxElement::ConstAccessor< iplink_t  > acc_originalObject("originalObjectLink");
  static const SG::AuxElement::ConstAccessor< iplink_t  > acc_nominalObject("nominalObjectLink");
  static const SG::AuxElement::ConstAccessor< std::vector<iplink_t > > acc_ghostMuons("GhostMuon");

  static const SG::AuxElement::ConstAccessor<float> acc_Eloss("EnergyLoss");

  static const SG::AuxElement::Accessor< std::vector<iplink_t> > dec_constitObjLinks("ConstitObjectLinks");
  static const SG::AuxElement::Accessor< std::vector<float> > dec_constitObjWeights("ConstitObjectWeights");

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

  // Constructors
  ////////////////
  ColumnarMETMaker::ColumnarMETMaker(const std::string& name) :
    AsgTool(name),
    m_PVkey("PrimaryVertices"),
    m_trkseltool(""),
    m_JvtTool("", this)
  {
    //
    // Property declaration
    //
    declareProperty("JetJvtMomentName",   m_jetJvtMomentName   = "Jvt"               );
    declareProperty("JetRejectionDec",    m_jetRejectionDec    = ""                  );
    declareProperty("JetMinEFrac",        m_jetMinEfrac        = 0.0                 );
    declareProperty("JetMinWeightedPt",   m_jetMinWeightedPt   = 20.0e3              );
    declareProperty("JetConstitScaleMom", m_jetConstitScaleMom = "JetConstitScaleMomentum");
    declareProperty("CorrectJetPhi",      m_jetCorrectPhi      = false               );
    declareProperty("DoPFlow",            m_doPFlow            = false               );
    declareProperty("DoSoftTruth",        m_doSoftTruth        = false               );
    declareProperty("DoJetTruth",         m_doConstJet         = false               );

    declareProperty("JetSelection",       m_jetSelection       = "Tight"             );
    declareProperty("JetEtaMax",          m_JetEtaMax          = 4.5                 );
    declareProperty("JetEtaForw",         m_JetEtaForw         = 2.5                 );
    declareProperty("CustomCentralJetPt", m_customCenJetPtCut  = 20e3                );
    declareProperty("CustomForwardJetPt", m_customFwdJetPtCut  = 20e3                );
    declareProperty("CustomJetJvtPtMax",  m_customJvtPtMax     = 60e3                );
    declareProperty("CustomJetJvtWP",     m_customJvtWP        = "FixedEffPt"        );

    declareProperty("DoMuonEloss",        m_muEloss            = false               );
    declareProperty("ORCaloTaggedMuons",  m_orCaloTaggedMuon   = true                );
    declareProperty("GreedyPhotons",      m_greedyPhotons      = false               );
    declareProperty("VeryGreedyPhotons",  m_veryGreedyPhotons  = false               );

    declareProperty("UseGhostMuons",      m_useGhostMuons      = false               );
    declareProperty("DoRemoveMuonJets",   m_doRemoveMuonJets   = true                );
    declareProperty("DoSetMuonJetEMScale", m_doSetMuonJetEMScale = true              );

    declareProperty("DoRemoveElecTrks",   m_doRemoveElecTrks   = true                );
    declareProperty("DoRemoveElecTrksEM", m_doRemoveElecTrksEM = false               );

    declareProperty("skipSystematicJetSelection", m_skipSystematicJetSelection = false,
            "EXPERIMENTAL: whether to use simplified OR based on nominal jets "
            "and for jet-related systematics only. "
            "WARNING: this property is strictly for doing physics studies of the feasibility "
            "of this OR scheme, it should not be used in a regular analysis");

    // muon overlap variables (expert use only)
    declareProperty("JetTrkNMuOlap",      m_jetTrkNMuOlap = 5                        );
    declareProperty("JetWidthMuOlap",     m_jetWidthMuOlap = 0.1                     );
    declareProperty("JetPsEMuOlap",       m_jetPsEMuOlap = 2.5e3                     );
    declareProperty("JetEmfMuOlap",       m_jetEmfMuOlap = 0.9                       );
    declareProperty("JetTrkPtMuPt",       m_jetTrkPtMuPt = 0.8                       );
    declareProperty("muIDPTJetPtRatioMuOlap", m_muIDPTJetPtRatioMuOlap = 2.0         );

    declareProperty("MissingObjWarnThreshold", m_missObjWarningPtThreshold = 7.0e3   );

    declareProperty("TrackSelectorTool",  m_trkseltool                               );
    declareProperty("JvtSelTool",         m_JvtTool                                  );
  }

  // Destructor
  ///////////////
  ColumnarMETMaker::~ColumnarMETMaker()
  = default;

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode ColumnarMETMaker::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    ATH_MSG_INFO("Use jet selection criterion: " << m_jetSelection << " PFlow: " << m_doPFlow);
    if (m_jetSelection == "Loose")       { m_CenJetPtCut = 20e3; m_FwdJetPtCut = 20e3; m_JvtWP = "FixedEffPt"; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tight")  { m_CenJetPtCut = 20e3; m_FwdJetPtCut = 30e3; m_JvtWP = "FixedEffPt"; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tighter"){ m_CenJetPtCut = 20e3; m_FwdJetPtCut = 35e3; m_JvtWP = "FixedEffPt"; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tenacious"){ m_CenJetPtCut = 20e3; m_FwdJetPtCut = 40e3; m_JvtWP = "FixedEffPt"; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tier0")  { m_CenJetPtCut = 0;    m_FwdJetPtCut = 0; m_JvtWP = "None"; }
    else if (m_jetSelection == "Expert")  {
      ATH_MSG_INFO("Custom jet selection configured. *** FOR EXPERT USE ONLY ***");
      m_CenJetPtCut = m_customCenJetPtCut;
      m_FwdJetPtCut = m_customFwdJetPtCut;
      m_JvtPtMax = m_customJvtPtMax;
      m_JvtWP = m_customJvtWP;
    }
    else if (m_jetSelection == "HRecoil")  {
      ATH_MSG_INFO("Jet selection for hadronic recoil calculation is configured.");
      m_CenJetPtCut = 9999e3;
      m_FwdJetPtCut = 9999e3;
      m_JetEtaMax   = 5;
      m_JvtWP       = "None";
    }
    else {
      if (m_jetSelection == "Default") ATH_MSG_WARNING( "WARNING:  Default is now deprecated" );
      ATH_MSG_ERROR( "Error: No available jet selection found! Please update JetSelection in ColumnarMETMaker. Choose one: Loose, Tight (recommended), Tighter, Tenacious" );
      return StatusCode::FAILURE;
    }

    if (!m_trkseltool.empty()) ATH_CHECK( m_trkseltool.retrieve() );

    if (m_JvtWP != "None"){
      if (m_JvtTool.empty()) {
        asg::AsgToolConfig config_jvt ("CP::NNJvtSelectionTool/JvtSelTool");
        ATH_CHECK(config_jvt.setProperty("WorkingPoint", m_JvtWP));
        ATH_CHECK(config_jvt.setProperty("JvtMomentName", "NNJvt"));
        ATH_CHECK(config_jvt.setProperty("MaxPtForJvt", m_JvtPtMax));
        ATH_CHECK(config_jvt.makePrivateTool(m_JvtTool));
      }
      ATH_CHECK(m_JvtTool.retrieve());
    }

    // ReadHandleKey(s)
    ATH_CHECK( m_PVkey.initialize() );

    // configurable accessors
    if (!m_jetRejectionDec.empty()) {
      m_acc_jetRejectionDec.emplace (*this, m_jetRejectionDec);
      ATH_MSG_INFO("Applying additional jet rejection criterium in MET calculation: " << m_jetRejectionDec);
    }

    ATH_MSG_INFO("Suppressing warnings of objects missing in METAssociationMap for objects with pT < " << m_missObjWarningPtThreshold/1e3 << " GeV.");

    // overlap removal simplification?
    if (m_skipSystematicJetSelection) {
      ATH_MSG_INFO("Requesting simplified overlap removal procedure in MET calculation");
    }
    
    if (!m_inputPreselectionName.empty()) {
      m_inputPreselectionAcc.emplace(*this, m_inputPreselectionName);
    }
    if (!m_jetConstitScaleMom.empty()) {
      m_jetConstitScaleMomAcc.emplace(*this, m_jetConstitScaleMom);
      m_jetConstitScaleMomFixedAcc.emplace(*this, "JetConstitScaleMomentum");
    }
    return StatusCode::SUCCESS;
  }


  // **** Rebuild generic MET term ****

  StatusCode ColumnarMETMaker::rebuildMET(const std::string& metKey,
                                  xAOD::Type::ObjectType metType,
                                  xAOD::MissingETContainer* metCont,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy objScale) const
  {
    if (!metCont)
    {
      ATH_MSG_ERROR("No MET container provided");
      return StatusCode::FAILURE;
    }
    if (!collection)
    {
      ATH_MSG_ERROR("No input collection provided for MET term \"" << metKey << "\"");
      return StatusCode::FAILURE;
    }

    return rebuildMET(metKey,metType,columnar::MutableMetRange (*metCont),columnar::ParticleRange (*collection),m_assocAcc(helper),objScale);
  }

  StatusCode ColumnarMETMaker::rebuildMET(const std::string& metKey,
                                  xAOD::Type::ObjectType metType,
                                  columnar::MutableMetRange metCont,
                                  columnar::ParticleRange collection,
                                  columnar::MetAssociationHelper<> helper,
                                  MissingETBase::UsageHandler::Policy objScale) const
  {
    MissingETBase::Types::bitmask_t metSource;
    switch(metType) {
    case xAOD::Type::Electron:
      metSource = MissingETBase::Source::electron();
      break;
    case xAOD::Type::Photon:
      metSource = MissingETBase::Source::photon();
      break;
    case xAOD::Type::Tau:
      metSource = MissingETBase::Source::tau();
      break;
    case xAOD::Type::Muon:
      metSource = MissingETBase::Source::muon();
      break;
    case xAOD::Type::Jet:
      ATH_MSG_WARNING("Incorrect use of rebuildMET -- use rebuildJetMET for RefJet term");
      return StatusCode::FAILURE;
    default:
      ATH_MSG_WARNING("Invalid object type provided: " << metType);
      return StatusCode::FAILURE;
    }

    columnar::MutableMetId met = m_outputMetMapAcc.fillMET (metCont, metKey, metSource);

    // If muon eloss corrections are required, create a new term to hold these if it doesn't already exist
    if(metType==xAOD::Type::Muon && (m_muEloss || m_doSetMuonJetEMScale)) {
      if (m_outputMetMapAcc.tryCreateIfMissing (metCont, "MuonEloss", MissingETBase::Source::Type::Muon | MissingETBase::Source::Category::Calo) != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("failed to create Muon Eloss MET term");
        return StatusCode::FAILURE;
      }
    }

    return rebuildMET(met,collection,helper,objScale);
  }

  StatusCode ColumnarMETMaker::rebuildMET(xAOD::MissingET* met,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy objScale) const
  {
    if (!met)
    {
      ATH_MSG_ERROR("No MET object provided");
      return StatusCode::FAILURE;
    }
    if (!collection)
    {
      ATH_MSG_ERROR("No input collection provided for MET term \"" << met->name() << "\"");
      return StatusCode::FAILURE;
    }

    return rebuildMET(columnar::MutableMetId(*met), columnar::ParticleRange(*collection), m_assocAcc(helper), objScale);
  }

  StatusCode ColumnarMETMaker::rebuildMET(columnar::MutableMetId met,
                                  columnar::ParticleRange collection,
                                  columnar::MetAssociationHelper<> helper,
                                  MissingETBase::UsageHandler::Policy objScale) const
  {
    MissingETBase::UsageHandler::Policy p = MissingETBase::UsageHandler::OnlyCluster;
    bool removeOverlap = true;
    if (m_inputObjTypeAcc(collection) == xAOD::Type::Muon) {
        p = MissingETBase::UsageHandler::OnlyTrack;
        removeOverlap = false;
    }
    if (m_doSoftTruth) p = MissingETBase::UsageHandler::TruthParticle;
    if (m_doPFlow) p = MissingETBase::UsageHandler::ParticleFlow;
    return rebuildMET(met,collection,helper,p,removeOverlap,objScale);
  }

  StatusCode ColumnarMETMaker::rebuildMET(xAOD::MissingET* met,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy p,
                                  bool removeOverlap,
                                  MissingETBase::UsageHandler::Policy objScale) const {
    if(!met || !collection) {
      ATH_MSG_ERROR("Invalid pointer supplied for "
                      << "MET (" << met << ") or "
                      << "collection (" << collection << ").");
      return StatusCode::FAILURE;
    }

    return rebuildMET(columnar::MutableMetId(*met),columnar::ParticleRange (*collection),m_assocAcc(helper),p,removeOverlap,objScale);
  }

  StatusCode ColumnarMETMaker::rebuildMET(columnar::MutableMetId met,
                                  columnar::ParticleRange collection,
                                  columnar::MetAssociationHelper<> helper,
                                  MissingETBase::UsageHandler::Policy p,
                                  bool removeOverlap,
                                  MissingETBase::UsageHandler::Policy objScale) const {
    if(helper.map().empty()) {
      ATH_MSG_WARNING("Incomplete association map received. Cannot rebuild MET.");
      ATH_MSG_WARNING("Note: ColumnarMETMaker should only be run on events containing at least one PV");
      return StatusCode::SUCCESS;
    }
    ATH_MSG_VERBOSE("Building MET term " << met(m_outputMetNameAcc));
    columnar::MetHelpers::ObjectWeightHandle<> metWeights(*this,m_outputMetWeightDecRegular,met,collection);

    if(collection.empty()) return StatusCode::SUCCESS;
    columnar::MetHelpers::OriginalObjectHandle collectionOriginals(*this,collection);

    if(collectionOriginals.isShallowCopy() && collectionOriginals.originalInputs()) {
      ATH_MSG_WARNING("Shallow copy provided without \"originalObjectLinks\" decoration! "
                      << "Overlap removal cannot be done. "
                      << "Will not compute this term.");
      ATH_MSG_WARNING("Please apply xAOD::setOriginalObjectLink() from xAODBase/IParticleHelpers.h");
      return StatusCode::SUCCESS;
    }
    ATH_MSG_VERBOSE("Original inputs? " << collectionOriginals.originalInputs());
    for(const auto obj : collection) {
      if (m_inputPreselectionAcc&&!(*m_inputPreselectionAcc)(obj)) continue;
      bool selected = false;
      auto orig = collectionOriginals.getOriginal(obj);
      auto assocs = helper.getAssociations(orig);
      if(assocs.empty()) {
        std::string message = "Object is not in association map. Did you make a deep copy but fail to set the \"originalObjectLinks\" decoration? "
                              "If not, Please apply xAOD::setOriginalObjectLink() from xAODBase/IParticleHelpers.h";
        // Avoid warnings for leptons with pT below threshold for association map
        if (m_inputMomAcc.pt(orig)>m_missObjWarningPtThreshold) {
            ATH_MSG_WARNING(message);
        } else {
            ATH_MSG_DEBUG(message);
        }
        // if this is an uncalibrated electron below the threshold, then we put it into the soft term
        if(m_inputObjTypeAcc(orig)==xAOD::Type::Electron){
          metWeights.emplace_back( obj, 0 );
          message = "Missing an electron from the MET map. Included as a track in the soft term. pT: " + std::to_string(m_inputMomAcc.pt(obj)/1e3) + " GeV";
          if (m_inputMomAcc.pt(orig)>m_missObjWarningPtThreshold) {
              ATH_MSG_WARNING(message);
          } else {
              ATH_MSG_DEBUG(message);
          }
          continue;
        } else {
          ATH_MSG_ERROR("Missing an object: " << m_inputObjTypeAcc(orig) << " pT: " << m_inputMomAcc.pt(obj)/1e3 << " GeV, may be duplicated in the soft term.");
        }
      }

      // If the object has already been selected and processed, ignore it.
      if(helper.objSelected(orig)) continue;
      selected = helper.selectIfNoOverlaps(orig,p) || !removeOverlap;
      ATH_MSG_VERBOSE(m_inputObjTypeAcc(obj) << " (" << orig <<") with pt " << m_inputMomAcc.pt(obj)
                      << " is " << ( selected ? "non-" : "") << "overlapping");

      // Greedy photon options: set selection flags
      if ((m_greedyPhotons || m_veryGreedyPhotons) && selected && m_inputObjTypeAcc(obj) == xAOD::Type::Photon){
        for(auto assoc : assocs){
          auto indices = m_assocAcc.overlapIndices(assoc,orig);
          auto allObjects = m_assocAcc.objects(assoc);
          for (size_t index : indices){
            const xAOD::IParticle* thisObj = allObjects[index].getXAODObject();
            if(!thisObj) continue;
            if ((thisObj->type() == xAOD::Type::Jet && m_veryGreedyPhotons) ||
                  thisObj->type() == xAOD::Type::Electron)
              helper.setObjSelectionFlag(assoc, thisObj, true);
          }
        }
      }

      //Do special overlap removal for calo tagged muons
      if(m_orCaloTaggedMuon && !removeOverlap && m_inputObjTypeAcc(orig)==xAOD::Type::Muon && m_inputMuonTypeAcc.getOptional(orig)==xAOD::Muon::CaloTagged) {
        for (decltype(auto) assoc : assocs) {
          auto ind = m_assocAcc.overlapIndices(assoc,orig);
          auto allObjects = m_assocAcc.objects(assoc);
          for (size_t indi = 0; indi < ind.size(); indi++) if (allObjects[ind[indi]]) {
              if (allObjects[ind[indi]].isContainer<columnar::ContainerId::electron>()
                  && helper.objSelected(assoc, ind[indi])) {
                selected = false;
                break;
              }
            }
        }
      }
      // Don't overlap remove muons, but flag the non-overlapping muons to take out their tracks from jets
      // Removed eloss from here -- clusters already flagged.
      // To be handled in rebuildJetMET
      if(selected) {
        if(objScale==MissingETBase::UsageHandler::PhysicsObject) {
          ATH_MSG_VERBOSE("Add object with pt " << m_inputMomAcc.pt(obj));
          m_outputMetMomAcc.addParticle (met, m_inputMomAcc, obj);
        } else {
          MissingETBase::Types::constvec_t constvec = helper.getConstVec(obj,objScale);
          ATH_MSG_VERBOSE("Add truth object with pt " << constvec.cpt());
          m_outputMetMomAcc.addParticle (met, constvec.cpx(),constvec.cpy(),constvec.cpt());
        }
        metWeights.emplace_back( obj, 1. );
      }
    }
    ATH_MSG_DEBUG("Built met term " << met(m_outputMetNameAcc) << ", with magnitude " << m_outputMetMomAcc.met(met));
    return StatusCode::SUCCESS;
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softKey,
                                     xAOD::MissingETContainer* metCont,
                                     const xAOD::JetContainer* jets,
                                     const xAOD::MissingETContainer* metCoreCont,
                                     xAOD::MissingETAssociationHelper& helper,
                                     bool doJetJVT) const
  {
    if (!metCont || !metCoreCont)
    {
      ATH_MSG_ERROR("No MET container provided");
      return StatusCode::FAILURE;
    }
    if (!jets)
    {
      ATH_MSG_ERROR("No input collection provided for MET term \"" << metJetKey << "\" and soft term: " << softKey);
      return StatusCode::FAILURE;
    }

    return rebuildJetMET(metJetKey,softKey,columnar::MutableMetRange(*metCont),columnar::JetRange(*jets),columnar::Met1Range(*metCoreCont),m_assocAcc(helper),doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softKey,
                                     columnar::MutableMetRange metCont,
                                     columnar::JetRange jets,
                                     columnar::Met1Range metCoreCont,
                                     columnar::MetAssociationHelper<> helper,
                                     bool doJetJVT) const
  {
    ATH_MSG_VERBOSE("Rebuild jet term: " << metJetKey << " and soft term: " << softKey);

    columnar::MutableMetId metJet {m_outputMetMapAcc.fillMET (metCont, metJetKey, MissingETBase::Source::jet())};

    columnar::OptMet1Id coreSoftClus, coreSoftTrk;
    columnar::OptMutableMetId metSoftClus, metSoftTrk;

    columnar::OptMet1Id coreSoft = m_inputMetMapAcc(metCoreCont,softKey+"Core");
    if(!coreSoft) {
      ATH_MSG_WARNING("Invalid soft term key supplied: " << softKey);
      return StatusCode::FAILURE;
    }
    if(MissingETBase::Source::isTrackTerm(coreSoft.value()(m_inputMetSourceAcc))) {
      coreSoftTrk = coreSoft;

      metSoftTrk = m_outputMetMapAcc.fillMET (metCont, softKey, coreSoftTrk.value()(m_inputMetSourceAcc));
    } else {
      coreSoftClus = coreSoft;

      metSoftClus = m_outputMetMapAcc.fillMET (metCont, softKey, coreSoftClus.value()(m_inputMetSourceAcc));
    }

    return rebuildJetMET(metJet, metCont, jets, helper,
                         metSoftClus, coreSoftClus,
                         metSoftTrk,  coreSoftTrk,
                         doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildTrackMET(const std::string& metJetKey,
                                       const std::string& softKey,
                                       xAOD::MissingETContainer* metCont,
                                       const xAOD::JetContainer* jets,
                                       const xAOD::MissingETContainer* metCoreCont,
                                       xAOD::MissingETAssociationHelper& helper,
                                       bool doJetJVT) const
  {
    if (!metCont)
    {
      ATH_MSG_ERROR("No MET container provided");
      return StatusCode::FAILURE;
    }
    if (!jets)
    {
      ATH_MSG_ERROR("No input collection provided for MET term \"" << metJetKey << "\" and soft term: " << softKey);
      return StatusCode::FAILURE;
    }

    return rebuildTrackMET(metJetKey,softKey,columnar::MutableMetRange(*metCont),columnar::JetRange(*jets),columnar::Met1Range(*metCoreCont),m_assocAcc(helper),doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildTrackMET(const std::string& metJetKey,
                                       const std::string& softKey,
                                       columnar::MutableMetRange metCont,
                                       columnar::JetRange jets,
                                       columnar::Met1Range metCoreCont,
                                       columnar::MetAssociationHelper<> helper,
                                       bool doJetJVT) const
  {
    ATH_MSG_VERBOSE("Rebuild jet term: " << metJetKey << " and soft term: " << softKey);

    columnar::MutableMetId metJet {m_outputMetMapAcc.fillMET (metCont, metJetKey, MissingETBase::Source::jet() | MissingETBase::Source::track())};

    columnar::OptMet1Id coreSoft = m_inputMetMapAcc(metCoreCont,softKey+"Core");
    if(!coreSoft) {
      ATH_MSG_WARNING("Invalid soft term key supplied: " << softKey);
      return StatusCode::FAILURE;
    }
    auto coreSoftTrk = coreSoft.value();

    columnar::MutableMetId metSoftTrk {m_outputMetMapAcc.fillMET (metCont, softKey, coreSoftTrk(m_inputMetSourceAcc))};

    return rebuildTrackMET(metJet, metCont, jets, helper,
                           metSoftTrk,  coreSoftTrk,
                           doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softClusKey,
                                     const std::string& softTrkKey,
                                     xAOD::MissingETContainer* metCont,
                                     const xAOD::JetContainer* jets,
                                     const xAOD::MissingETContainer* metCoreCont,
                                     xAOD::MissingETAssociationHelper& helper,
                                     bool doJetJVT) const
  {
    if (!metCont)
    {
      ATH_MSG_ERROR("No MET container provided");
      return StatusCode::FAILURE;
    }
    if (!jets)
    {
      ATH_MSG_ERROR("No input collection provided for MET term \"" << metJetKey << "\" and soft term: " << softClusKey << " and " << softTrkKey);
      return StatusCode::FAILURE;
    }

    return rebuildJetMET(metJetKey,softClusKey,softTrkKey,columnar::MutableMetRange(*metCont),columnar::JetRange(*jets),columnar::Met1Range(*metCoreCont),m_assocAcc(helper),doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softClusKey,
                                     const std::string& softTrkKey,
                                     columnar::MutableMetRange metCont,
                                     columnar::JetRange jets,
                                     columnar::Met1Range metCoreCont,
                                     columnar::MetAssociationHelper<> helper,
                                     bool doJetJVT) const
  {

    ATH_MSG_VERBOSE("Create Jet MET " << metJetKey);
    columnar::MutableMetId metJet {m_outputMetMapAcc.fillMET (metCont, metJetKey, MissingETBase::Source::jet())};
    ATH_MSG_VERBOSE("Create SoftClus MET " << softClusKey);
    auto coreSoftClus = m_inputMetMapAcc(metCoreCont,softClusKey+"Core");
    ATH_MSG_VERBOSE("Create SoftTrk MET " << softTrkKey);
    auto coreSoftTrk = m_inputMetMapAcc(metCoreCont,softTrkKey+"Core");
    if(!coreSoftClus) {
      ATH_MSG_WARNING("Invalid cluster soft term key supplied: " << softClusKey);
      return StatusCode::FAILURE;
    }
    if(!coreSoftTrk) {
      ATH_MSG_WARNING("Invalid track soft term key supplied: " << softTrkKey);
      return StatusCode::FAILURE;
    }
    columnar::MutableMetId metSoftClus {m_outputMetMapAcc.fillMET (metCont, softClusKey, m_inputMetSourceAcc(coreSoftClus.value()))};

    columnar::MutableMetId metSoftTrk {m_outputMetMapAcc.fillMET (metCont, softTrkKey, m_inputMetSourceAcc(coreSoftTrk.value()))};

    return rebuildJetMET(metJet, metCont, jets, helper,
                         metSoftClus, coreSoftClus,
                         metSoftTrk, coreSoftTrk,
                         doJetJVT);
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(xAOD::MissingET* metJet,
                                     const xAOD::JetContainer* jets,
                                     xAOD::MissingETAssociationHelper& helper,
                                     xAOD::MissingET* metSoftClus,
                                     const xAOD::MissingET* coreSoftClus,
                                     xAOD::MissingET* metSoftTrk,
                                     const xAOD::MissingET* coreSoftTrk,
                                     bool doJetJVT,
                                     bool tracksForHardJets,
                                     std::vector<const xAOD::IParticle*>* softConst) const {
    if(!metJet || !jets) {
      ATH_MSG_ERROR("Invalid pointer supplied for "
                      << "MET (" << metJet << ") or "
                      << "jet collection (" << jets << ").");
      return StatusCode::FAILURE;
    }
    columnar::MutableMetRange metCont (*static_cast<MissingETContainer*>(metJet->container()));
    return rebuildJetMET(columnar::MutableMetId(*metJet), metCont, columnar::JetRange(*jets), m_assocAcc(helper),
                         columnar::OptMutableMetId(metSoftClus), columnar::OptMet1Id (coreSoftClus),
                         columnar::OptMutableMetId(metSoftTrk), columnar::OptMet1Id (coreSoftTrk),
                         doJetJVT, tracksForHardJets, softConst);
  }

  StatusCode ColumnarMETMaker::rebuildJetMET(columnar::MutableMetId metJet,
                                     columnar::MutableMetRange metCont,
                                     columnar::JetRange jets,
                                     columnar::MetAssociationHelper<> helper,
                                     columnar::OptMutableMetId metSoftClus,
                                     columnar::OptMet1Id coreSoftClus,
                                     columnar::OptMutableMetId metSoftTrk,
                                     columnar::OptMet1Id coreSoftTrk,
                                     bool doJetJVT,
                                     bool tracksForHardJets,
                                     std::vector<const xAOD::IParticle*>* softConst) const {
    if(softConst && m_trkseltool.empty() && !m_doPFlow && !m_doSoftTruth) {
      ATH_MSG_WARNING( "Requested soft track element links, but no track selection tool supplied.");
    }
    const xAOD::Vertex *pv = softConst?getPV():nullptr;

    if(helper.map().empty()) {
      ATH_MSG_WARNING("Incomplete association map received. Cannot rebuild MET.");
      ATH_MSG_WARNING("Note: ColumnarMETMaker should only be run on events containing at least one PV");
      return StatusCode::SUCCESS;
    }

    if(doJetJVT && m_JvtWP == "None"){
      ATH_MSG_WARNING("rebuildJetMET requested JVT, which is inconsistent with jet selection " << m_jetSelection << ". Ignoring JVT.");
      doJetJVT = false;
    }

    ATH_MSG_VERBOSE("Building MET jet term " << metJet(m_outputMetNameAcc));
    if(!metSoftClus && !metSoftTrk) {
      ATH_MSG_WARNING("Neither soft cluster nor soft track term has been supplied!");
      return StatusCode::SUCCESS;
    }
    static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<IParticleContainer> > > acc_softConst("softConstituents");
    std::optional<columnar::MetHelpers::ObjectWeightHandle<columnar::ContainerId::mutableMet,columnar::ContainerId::jet>> metSoftClusLinks;
    if(metSoftClus) {
      metSoftClusLinks.emplace(*this,m_jetOutputMetWeightDecSoft,metSoftClus.value(),jets);
      if(!coreSoftClus) {
        ATH_MSG_ERROR("Soft cluster term provided without a core term!");
        return StatusCode::FAILURE;
      }
      ATH_MSG_VERBOSE("Building MET soft cluster term " << metSoftClus.value()(m_outputMetNameAcc));
      ATH_MSG_VERBOSE("Core soft cluster mpx " << m_inputMetMomAcc.mpx(coreSoftClus.value())
                      << ", mpy " << m_inputMetMomAcc.mpy(coreSoftClus.value())
                      << " sumet " << m_inputMetMomAcc.sumet(coreSoftClus.value()));
      m_outputMetMomAcc.addMet (metSoftClus.value(), m_inputMetMomAcc, coreSoftClus.value());
      // Fill a vector with the soft constituents, if one was provided.
      // For now, only setting up to work with those corresponding to the jet constituents.
      // Can expand if needed.
      if(softConst && acc_softConst.isAvailable(*coreSoftClus.getXAODObject())) {
        for(const auto& constit : acc_softConst(*coreSoftClus.getXAODObject())) {
          softConst->push_back(*constit);
        }
        ATH_MSG_DEBUG(softConst->size() << " soft constituents from core term");
      }
    }
    std::optional<columnar::MetHelpers::ObjectWeightHandle<columnar::ContainerId::mutableMet,columnar::ContainerId::jet>> metSoftTrkLinks;
    if(metSoftTrk) {
      metSoftTrkLinks.emplace(*this,m_jetOutputMetWeightDecSoft,metSoftTrk.value(),jets);
      if(!coreSoftTrk) {
        ATH_MSG_ERROR("Soft track term provided without a core term!");
        return StatusCode::FAILURE;
      }
      ATH_MSG_VERBOSE("Building MET soft track term " << metSoftTrk.value()(m_outputMetNameAcc));
      ATH_MSG_VERBOSE("Core soft track mpx " << m_inputMetMomAcc.mpx(coreSoftTrk.value())
                      << ", mpy " << m_inputMetMomAcc.mpy(coreSoftTrk.value())
                      << " sumet " << m_inputMetMomAcc.sumet(coreSoftTrk.value()));
      m_outputMetMomAcc.addMet (metSoftTrk.value(), m_inputMetMomAcc, coreSoftTrk.value());
      if(softConst && acc_softConst.isAvailable(*coreSoftTrk.getXAODObject()) && !m_doPFlow && !m_doSoftTruth) {
        for(const auto& constit : acc_softConst(*coreSoftTrk.getXAODObject())) {
          softConst->push_back(*constit);
        }
        ATH_MSG_DEBUG(softConst->size() << " soft constituents from trk core term");
      }
    }

    columnar::MetHelpers::ObjectWeightHandle<columnar::ContainerId::mutableMet,columnar::ContainerId::jet> metJetWeights(*this,m_jetOutputMetWeightDecRegular,metJet,jets);

    // Get the hashed key of this jet, if we can. Though his only works if
    //   1. the container is an owning container, and not just a view;
    //   2. the container is in the event store already.
    // Since we will be creating ElementLink-s to these jets later on in the
    // code, and it should work in AnalysisBase, only the first one of these
    // is checked. Since the code can not work otherwise.

    columnar::MetHelpers::OriginalObjectHandle<columnar::ContainerId::jet> jetsOriginals(*this,jets);
    for(auto jet : jets) {
      auto originalJet = jetsOriginals.getOriginal(jet);
      auto assoc = helper.getJetAssociation(originalJet);
      if(!assoc || m_assocAcc.isMisc(*assoc)) {
        ATH_MSG_WARNING( "Jet without association found!" );
        continue;
      }

      if(m_skipSystematicJetSelection) {
        // retrieve nominal calibrated jet
        if (acc_nominalObject.isAvailable(jet.getXAODObject())){
          ATH_MSG_VERBOSE( "Jet pt before nominal replacement = " << m_jetMomAcc.pt(jet));
          jet = *static_cast<const xAOD::Jet*>(*acc_nominalObject(jet.getXAODObject()));
        }
        else
          ATH_MSG_ERROR("No nominal calibrated jet available for jet " << jet << ". Cannot simplify overlap removal!");
      }
      ATH_MSG_VERBOSE( "Jet pt = " << m_jetMomAcc.pt(jet));

      bool selected = (std::abs(m_jetMomAcc.eta(jet))<m_JetEtaForw && m_jetMomAcc.pt(jet)>m_CenJetPtCut) || (std::abs(m_jetMomAcc.eta(jet))>=m_JetEtaForw && m_jetMomAcc.pt(jet)>m_FwdJetPtCut );
      bool JVT_reject(false);
      bool isMuFSRJet(false);

      // Apply a cut on the maximum jet eta. This restricts jets to those with calibration. Excluding more forward jets was found to have a minimal impact on the MET in Zee events
      if (m_JetEtaMax > 0.0 && std::abs(m_jetMomAcc.eta(jet)) > m_JetEtaMax)
        JVT_reject = true;

      if(doJetJVT) {
        // intrinsically checks that is within range to apply Jvt requirement
        JVT_reject  = !bool(m_JvtTool->accept(&jet.getXAODObject()));
        ATH_MSG_VERBOSE("Jet " << (JVT_reject ? "fails" : "passes") <<" JVT selection");
      }

      // if defined apply additional jet criterium
      if (m_acc_jetRejectionDec && (*m_acc_jetRejectionDec)(jet)==0) JVT_reject = true;
      bool hardJet(false);
      MissingETBase::Types::constvec_t calvec = helper.overlapCalVec(*assoc);
      bool caloverlap = false;
      caloverlap = calvec.ce()>0;
      ATH_MSG_DEBUG("Jet " << jet << " is " << ( caloverlap ? "" : "non-") << "overlapping");

      if(m_veryGreedyPhotons && caloverlap) {
        for(const auto object : m_assocAcc.objects(*assoc)) {
          // Correctly handle this jet if we're using very greedy photons
          if (object && object.getXAODObject()->type() == xAOD::Type::Photon) hardJet = true;
        }
      }

      xAOD::JetFourMom_t constjet;
      double constSF(1);
      if(m_jetConstitScaleMom.empty() && m_assocAcc.hasAlternateConstVec(*assoc)){
        constjet = m_assocAcc.getAlternateConstVec(*assoc);
      } else {
        constjet = m_jetConstitScaleMomAcc.value().jetP4(jet);//grab a constituent scale added by the JetMomentTool/JetConstitFourMomTool.cxx
        double denom = (m_assocAcc.hasAlternateConstVec(*assoc) ? m_assocAcc.getAlternateConstVec(*assoc) : m_jetConstitScaleMomFixedAcc.value().jetP4(jet)).E();
        constSF = denom>1e-9 ? constjet.E()/denom : 0.;
        ATH_MSG_VERBOSE("Scale const jet by factor " << constSF);
        calvec *= constSF;
      }
      double jpx = constjet.Px();
      double jpy = constjet.Py();
      double jpt = constjet.Pt();
      double opx = jpx - calvec.cpx();
      double opy = jpy - calvec.cpy();

      columnar::OptMutableMetId met_muonEloss;
      if(m_muEloss || m_doSetMuonJetEMScale) {
        // Get a term to hold the Eloss corrections
        met_muonEloss = m_outputMetMapAcc.getRequired(metCont, "MuonEloss");
        if(!met_muonEloss) {
          ATH_MSG_WARNING("Attempted to apply muon Eloss correction, but corresponding MET term does not exist!");
          return StatusCode::FAILURE;
        }
      }

      float total_eloss(0);
      MissingETBase::Types::bitmask_t muons_selflags(0);
      std::vector<columnar::MuonId> muons_in_jet;
      std::vector<columnar::ElectronId> electrons_in_jet;
      bool passJetForEl=false;
      if(m_useGhostMuons) { // for backwards-compatibility
        if(!acc_ghostMuons.isAvailable(jet.getXAODObject())){
          ATH_MSG_ERROR("Ghost muons requested but not found!");
          return StatusCode::FAILURE;
        }
        for(const auto& el : acc_ghostMuons(jet.getXAODObject())) {
          if(!el.isValid()){
            ATH_MSG_ERROR("Invalid element link to ghost muon! Quitting.");
            return StatusCode::FAILURE;
          }
          muons_in_jet.push_back(*static_cast<const xAOD::Muon*>(*el));
        }
      }
      for(const auto obj : m_assocAcc.objects(*assoc)) {
        if(!obj) continue;
        if(!m_useGhostMuons && obj.isContainer<columnar::ContainerId::muon>()) {
          auto mu_test = obj.tryGetVariant<columnar::ContainerId::muon>().value();
          ATH_MSG_VERBOSE("Muon " << mu_test << " found in jet " << jet);
          if((m_doRemoveMuonJets || m_doSetMuonJetEMScale)) {
            if constexpr (columnar::ColumnarModeDefault::isXAOD) {
              if(acc_originalObject.isAvailable(mu_test.getXAODObject())) mu_test = *static_cast<const xAOD::Muon*>(*acc_originalObject(mu_test.getXAODObject()));
            }
            if(helper.objSelected(mu_test)) { //
              muons_in_jet.push_back(mu_test);
              ATH_MSG_VERBOSE("Muon is selected by MET.");
            }
          }
        } else if(m_doRemoveElecTrks && obj.isContainer<columnar::ContainerId::electron>()) {
          auto el_test = obj.tryGetVariant<columnar::ContainerId::electron>().value();
          ATH_MSG_VERBOSE("Electron " << el_test << " found in jet " << jet);
          if constexpr (columnar::ColumnarModeDefault::isXAOD) {
            if(acc_originalObject.isAvailable(el_test.getXAODObject())) el_test = *static_cast<const xAOD::Electron*>(*acc_originalObject(el_test.getXAODObject()));
          }
          if(helper.objSelected(*assoc,el_test)){
            if(el_test(m_electronPtAcc)>90.0e3) { // only worry about high-pt electrons?
              electrons_in_jet.push_back(el_test);
              ATH_MSG_VERBOSE("High-pt electron is selected by MET.");
            }
          }
        }
      }
      if(m_doRemoveElecTrks) {
        MissingETBase::Types::constvec_t initialTrkMom = m_assocAcc.jetTrkVec(*assoc);
        float jet_ORtrk_sumpt = helper.overlapTrkVec(*assoc).sumpt();
        float jet_all_trk_pt =  initialTrkMom.sumpt();
        float jet_unique_trk_pt = jet_all_trk_pt - jet_ORtrk_sumpt;
        MissingETBase::Types::constvec_t el_calvec;
        MissingETBase::Types::constvec_t el_trkvec;
        for(const auto& elec : electrons_in_jet) {
            el_calvec += m_assocAcc.calVec(*assoc,elec);
            el_trkvec += m_assocAcc.trkVec(*assoc,&elec.getXAODObject());
        }
        float el_cal_pt = el_calvec.cpt();
        float el_trk_pt = el_trkvec.cpt();
        ATH_MSG_VERBOSE("Elec trk: " << el_trk_pt
                        << " jetalltrk: " << jet_all_trk_pt
                        << " jetORtrk: " << jet_ORtrk_sumpt
                        << " electrk-jetORtrk: " << (el_trk_pt-jet_ORtrk_sumpt)
                        << " elec cal: " << el_cal_pt
                        << " jetalltrk-electrk: " << (jet_all_trk_pt-el_trk_pt)
                        << " jetalltrk-jetORtrk: " << (jet_all_trk_pt-jet_ORtrk_sumpt) );
        // Want to use the jet calo measurement if we had at least one electron
        // and the jet has a lot of residual track pt
        // Is the cut appropriate?
        if(el_trk_pt>1e-9 && jet_unique_trk_pt>10.0e3) passJetForEl=true;
      } // end ele-track removal

      for(auto mu_in_jet : muons_in_jet) {
        float mu_Eloss = acc_Eloss(mu_in_jet.getXAODObject());

        if(!JVT_reject) {
          if (m_doRemoveMuonJets) {
            // need to investigate how this is affected by the recording of muon clusters in the map
            float mu_id_pt = mu_in_jet.getXAODObject().trackParticle(xAOD::Muon::InnerDetectorTrackParticle) ? mu_in_jet.getXAODObject().trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt() : 0.;
            float jet_trk_sumpt = m_acc_trksumpt.isAvailable(jet) && this->getPV() ? m_acc_trksumpt(jet)[this->getPV()->index()] : 0.;

            // missed the muon, so we should add it back
            if(0.9999*mu_id_pt>jet_trk_sumpt)
              jet_trk_sumpt+=mu_id_pt;
            float jet_trk_N = m_acc_trkN.isAvailable(jet) && this->getPV() ? m_acc_trkN(jet)[this->getPV()->index()] : 0.;
            ATH_MSG_VERBOSE("Muon has ID pt " << mu_id_pt);
            ATH_MSG_VERBOSE("Jet has pt " << m_jetMomAcc.pt(jet) << ", trk sumpt " << jet_trk_sumpt << ", trk N " << jet_trk_N);
            bool jet_from_muon = mu_id_pt>1e-9 && jet_trk_sumpt>1e-9 && (m_jetMomAcc.pt(jet)/mu_id_pt < m_muIDPTJetPtRatioMuOlap && mu_id_pt/jet_trk_sumpt>m_jetTrkPtMuPt) && jet_trk_N<m_jetTrkNMuOlap;
            if(jet_from_muon) {
              ATH_MSG_VERBOSE("Jet is from muon -- remove.");
              JVT_reject = true;
            }
          }

          if (m_doSetMuonJetEMScale) {
            // need to investigate how this is affected by the recording of muon clusters in the map
            float mu_id_pt = mu_in_jet.getXAODObject().trackParticle(xAOD::Muon::InnerDetectorTrackParticle) ? mu_in_jet.getXAODObject().trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt() : 0.;
            float jet_trk_sumpt = m_acc_trksumpt.isAvailable(jet) && this->getPV() ? m_acc_trksumpt(jet)[this->getPV()->index()] : 0.;
            // missed the muon, so we should add it back
            if(0.9999*mu_id_pt>jet_trk_sumpt)
              jet_trk_sumpt+=mu_id_pt;
            float jet_trk_N = m_acc_trkN.isAvailable(jet) && this->getPV() ? m_acc_trkN(jet)[this->getPV()->index()] : 0.;

            float jet_psE = 0.;
            if (m_acc_psf.isAvailable(jet)){
              jet_psE = m_acc_psf(jet);
            } else if (m_acc_sampleE.isAvailable(jet)){
              jet_psE = m_acc_sampleE(jet)[0] + m_acc_sampleE(jet)[4];
            } else {
              ATH_MSG_ERROR("Jet PS fraction or sampling energy must be available to calculate MET with doSetMuonJetEMScale");
              return StatusCode::FAILURE;
            }

            bool jet_from_muon = jet_trk_sumpt>1e-9 && jet_trk_N<3 && mu_id_pt / jet_trk_sumpt > m_jetTrkPtMuPt && m_acc_emf(jet)>m_jetEmfMuOlap && m_acc_width(jet)<m_jetWidthMuOlap && jet_psE>m_jetPsEMuOlap;
            ATH_MSG_VERBOSE("Muon has ID pt " << mu_id_pt);
            ATH_MSG_VERBOSE("Jet has trk sumpt " << jet_trk_sumpt << ", trk N " << jet_trk_N << ", PS E " << jet_psE << ", width " << m_acc_width(jet) << ", emfrac " << m_acc_emf(jet));

            if(jet_from_muon) {
              ATH_MSG_VERBOSE("Jet is from muon -- set to EM scale and subtract Eloss.");
              // Using constjet now because we focus on AntiKt4EMTopo.
              // Probably not a massive difference to LC, but PF needs some consideration
              ATH_MSG_VERBOSE("Jet e: " << constjet.E() << ", mu Eloss: " << mu_Eloss);
              float elosscorr = mu_Eloss >= constjet.e() ? 0. : 1.-mu_Eloss/constjet.e();
              // Effectively, take the unique fraction of the jet times the eloss-corrected fraction
              // This might in some cases oversubtract, but should err on the side of undercounting the jet contribution
              opx *= elosscorr;
              opy *= elosscorr;
              ATH_MSG_VERBOSE(" Jet eloss factor " << elosscorr << ", final pt: " << sqrt(opx*opx+opy*opy));
              // Don't treat this jet normally. Instead, just add to the Eloss term
              isMuFSRJet = true;
            }
          }
        } // end muon-jet overlap-removal

        switch(mu_in_jet.getXAODObject().energyLossType()) {
          case xAOD::Muon::Parametrized:
          case xAOD::Muon::MOP:
          case xAOD::Muon::Tail:
          case xAOD::Muon::FSRcandidate:
          case xAOD::Muon::NotIsolated:
            // For now don't differentiate the behaviour
            // Remove the Eloss assuming the parameterised value
            // The correction is limited to the selected clusters
            total_eloss += mu_Eloss;
            muons_selflags |= (1<<m_assocAcc.findIndex(*assoc,mu_in_jet));
        }
      }
      ATH_MSG_VERBOSE("Muon selection flags: " << muons_selflags);
      ATH_MSG_VERBOSE("Muon total eloss: " << total_eloss);

      MissingETBase::Types::constvec_t mu_calovec;
      // borrowed from overlapCalVec
      for(size_t iKey = 0; iKey < m_assocAcc.sizeCal(*assoc); iKey++) {
        bool selector = (muons_selflags & m_assocAcc.calkey(*assoc)[iKey]);
        if(selector) mu_calovec += m_assocAcc.calVec(*assoc,iKey);
        ATH_MSG_VERBOSE("This key: " << m_assocAcc.calkey(*assoc)[iKey] << ", selector: " << selector);
      }
      ATH_MSG_VERBOSE("Mu calovec pt, no Eloss:   " << mu_calovec.cpt());
      if(m_muEloss) mu_calovec *= std::max<float>(0.,1-(total_eloss/mu_calovec.ce()));
      ATH_MSG_VERBOSE("Mu calovec pt, with Eloss: " << mu_calovec.cpt());

      // re-add calo components of muons beyond Eloss correction
      ATH_MSG_VERBOSE("Jet " << jet << " const pT before OR " << jpt);
      ATH_MSG_VERBOSE("Jet " << jet << " const pT after OR " << sqrt(opx*opx+opy*opy));
      opx += mu_calovec.cpx();
      opy += mu_calovec.cpy();
      double opt = sqrt( opx*opx+opy*opy );
      ATH_MSG_VERBOSE("Jet " << jet << " const pT diff after OR readding muon clusters " << opt-jpt);
      double uniquefrac = 1. - (calvec.ce() - mu_calovec.ce()) / constjet.E();
      ATH_MSG_VERBOSE( "Jet constscale px, py, pt, E = " << jpx << ", " << jpy << ", " << jpt << ", " << constjet.E() );
      ATH_MSG_VERBOSE( "Jet overlap E = " << calvec.ce() - mu_calovec.ce() );
      ATH_MSG_VERBOSE( "Jet OR px, py, pt, E = " << opx << ", " << opy << ", " << opt << ", " << constjet.E() - calvec.ce() );

      if(isMuFSRJet) {
        if(!met_muonEloss){
          ATH_MSG_ERROR("Attempted to apply muon Eloss correction, but corresponding MET term does not exist!");
          return StatusCode::FAILURE;
        }
        m_outputMetMomAcc.addParticle(met_muonEloss.value(),opx,opy,opt);
        continue;
      }

      if(selected && !JVT_reject) {
        if(!caloverlap) {
          // add jet full four-vector
          hardJet = true;
          if (!tracksForHardJets) {
            if(m_doConstJet)
              m_outputMetMomAcc.addParticle(metJet,jpx,jpy,jpt);
            else
              m_outputMetMomAcc.addParticle(metJet,m_jetMomAcc,jet);
          }
        }
        else if((uniquefrac>m_jetMinEfrac || passJetForEl) && opt>m_jetMinWeightedPt){
          // add jet corrected for overlaps if sufficient unique fraction
          hardJet = true;
          if(!tracksForHardJets) {
            if(m_jetCorrectPhi) {
              if (m_doConstJet)
                m_outputMetMomAcc.addParticle(metJet,opx,opy,opt);
              else {
                double jesF = m_jetMomAcc.pt(jet) / jpt;
                m_outputMetMomAcc.addParticle(metJet,opx*jesF,opy*jesF,opt*jesF);
              }
            } else {
              if (m_doConstJet)
                m_outputMetMomAcc.addParticle(metJet,uniquefrac*jpx,uniquefrac*jpy,uniquefrac*jpt);
              else{
                if(passJetForEl && m_doRemoveElecTrksEM)
                  m_outputMetMomAcc.addParticle(metJet,opx,opy,opt);
                else
                  m_outputMetMomAcc.addParticle(metJet,uniquefrac*m_jetMomAcc.px(jet),uniquefrac*m_jetMomAcc.py(jet),uniquefrac*m_jetMomAcc.pt(jet));
              }
            }
          }
        }
      }  // hard jet selection

      if(hardJet){
        ATH_MSG_VERBOSE("Jet added at full scale");
        metJetWeights.emplace_back(jet, uniquefrac);
      } else {
        if(metSoftClus && !JVT_reject) {
          // add fractional contribution
          ATH_MSG_VERBOSE("Jet added at const scale");
          if (std::abs(m_jetMomAcc.eta(jet))<2.5 || !(coreSoftClus.value()(m_inputMetSourceAcc)&MissingETBase::Source::Region::Central)) {
            metSoftClusLinks->emplace_back (jet, uniquefrac);
            m_outputMetMomAcc.addParticle(metSoftClus.value(),opx,opy,opt);
          }

          // Fill a vector with the soft constituents, if one was provided.
          // For now, only setting up to work with those corresponding to the jet constituents.
          // Can expand if needed.
          // This ignores overlap removal.
          //
          if(softConst) {
            for(size_t iConst=0; iConst<jet.getXAODObject().numConstituents(); ++iConst) {
              const IParticle* constit = jet.getXAODObject().rawConstituent(iConst);
              softConst->push_back(constit);
            }
          }
        }
      } // hard jet or CST

      if(!metSoftTrk || (hardJet && !tracksForHardJets)) continue;

      // use jet tracks
      // remove any tracks already used by other objects
      MissingETBase::Types::constvec_t trkvec = helper.overlapTrkVec(*assoc);
      MissingETBase::Types::constvec_t jettrkvec = m_assocAcc.jetTrkVec(*assoc);
      if(jettrkvec.ce()>1e-9) {
        jpx = jettrkvec.cpx();
        jpy = jettrkvec.cpy();
        jpt = jettrkvec.sumpt();
        jettrkvec -= trkvec;
        opx = jettrkvec.cpx();
        opy = jettrkvec.cpy();
        opt = jettrkvec.sumpt();
        ATH_MSG_VERBOSE( "Jet track px, py, sumpt = " << jpx << ", " << jpy << ", " << jpt );
        ATH_MSG_VERBOSE( "Jet OR px, py, sumpt = " << opx << ", " << opy << ", " << opt );
      } else {
        opx = opy = opt = 0;
        ATH_MSG_VERBOSE( "This jet has no associated tracks" );
      }
      if (hardJet) m_outputMetMomAcc.addParticle(metJet,opx,opy,opt);
      else if (std::abs(m_jetMomAcc.eta(jet))<2.5 || !(coreSoftTrk.value()(m_inputMetSourceAcc)&MissingETBase::Source::Region::Central)) {
        m_outputMetMomAcc.addParticle(metSoftTrk.value(),opx,opy,opt);
        // Don't need to add if already done for softclus.
        if(!metSoftClus) {
          if (metSoftTrkLinks) metSoftTrkLinks->emplace_back (jet, uniquefrac);
        }

        // Fill a vector with the soft constituents, if one was provided.
        // For now, only setting up to work with those corresponding to the jet constituents.
        // Can expand if needed.
        // This ignores overlap removal.
        //
        if(softConst && !m_doPFlow && !m_doSoftTruth) {
          std::vector<const IParticle*> jettracks;
          jet.getXAODObject().getAssociatedObjects<IParticle>(xAOD::JetAttribute::GhostTrack,jettracks);
          for(size_t iConst=0; iConst<jettracks.size(); ++iConst) {
            const TrackParticle* pTrk = static_cast<const TrackParticle*>(jettracks[iConst]);
            if (acceptTrack(pTrk,pv)) softConst->push_back(pTrk);
          }
        }
      }
    } // jet loop

    ATH_MSG_DEBUG("Number of selected jets: " << metJetWeights.size());

    if(metSoftTrk) {
      ATH_MSG_DEBUG("Number of softtrk jets: " << metSoftTrkLinks->size());
    }

    if(metSoftClus) {
      ATH_MSG_DEBUG("Number of softclus jets: " << metSoftClusLinks->size());
    }

    if(softConst) ATH_MSG_DEBUG(softConst->size() << " soft constituents from core term + jets");

    auto assoc = helper.getMiscAssociation();
    if(!assoc) return StatusCode::SUCCESS;

    if(metSoftTrk) {
      // supplement track term with any tracks associated to isolated muons
      // these are recorded in the misc association
      MissingETBase::Types::constvec_t trkvec = helper.overlapTrkVec(*assoc);
      double opx = trkvec.cpx();
      double opy = trkvec.cpy();
      double osumpt = trkvec.sumpt();
      ATH_MSG_VERBOSE( "Misc track px, py, sumpt = " << opx << ", " << opy << ", " << osumpt );
      m_outputMetMomAcc.addParticle(metSoftTrk.value(),opx,opy,osumpt);
      ATH_MSG_VERBOSE("Final soft track mpx " << m_outputMetMomAcc.mpx(metSoftTrk.value())
                      << ", mpy " << m_outputMetMomAcc.mpy(metSoftTrk.value())
                      << " sumet " << m_outputMetMomAcc.sumet(metSoftTrk.value()));
    }

    if(metSoftClus) {
      // supplement cluster term with any clusters associated to isolated e/gamma
      // these are recorded in the misc association
      float total_eloss(0.);
      MissingETBase::Types::bitmask_t muons_selflags(0);
      MissingETBase::Types::constvec_t calvec = helper.overlapCalVec(*assoc);
      double opx = calvec.cpx();
      double opy = calvec.cpy();
      double osumpt = calvec.sumpt();
      for(const auto objId : m_assocAcc.objects(*assoc)) {
        auto *obj = objId.getXAODObject();
        if (!obj || obj->type() != xAOD::Type::Muon) continue;
        const xAOD::Muon* mu_test(static_cast<const xAOD::Muon*>(obj));
        if(acc_originalObject.isAvailable(*mu_test)) mu_test = static_cast<const xAOD::Muon*>(*acc_originalObject(*mu_test));
        if(helper.objSelected(mu_test)) { //
          float mu_Eloss = acc_Eloss(*mu_test);
          switch(mu_test->energyLossType()) {
          case xAOD::Muon::Parametrized:
          case xAOD::Muon::MOP:
          case xAOD::Muon::Tail:
          case xAOD::Muon::FSRcandidate:
          case xAOD::Muon::NotIsolated:
            // For now don't differentiate the behaviour
            // Remove the Eloss assuming the parameterised value
            // The correction is limited to the selected clusters
            total_eloss += mu_Eloss;
            muons_selflags |= (1<<m_assocAcc.findIndex(*assoc,mu_test));
          }
          ATH_MSG_VERBOSE("Mu index " << mu_test->index());
        }
      }
      ATH_MSG_VERBOSE("Mu selection flags " << muons_selflags);
      ATH_MSG_VERBOSE("Mu total eloss " << total_eloss);

      MissingETBase::Types::constvec_t mu_calovec;
      // borrowed from overlapCalVec
      for(size_t iKey = 0; iKey < m_assocAcc.sizeCal(*assoc); iKey++) {
        bool selector = (muons_selflags & m_assocAcc.calkey(*assoc)[iKey]);
        ATH_MSG_VERBOSE("This key: " << m_assocAcc.calkey(*assoc)[iKey] << ", selector: " << selector
                        << " this calvec E: " << m_assocAcc.calVec(*assoc,iKey).ce());
        if(selector) mu_calovec += m_assocAcc.calVec(*assoc,iKey);
      }
      if(m_muEloss){
        mu_calovec *= std::max<float>(0.,1-(total_eloss/mu_calovec.ce()));
        opx += mu_calovec.cpx();
        opy += mu_calovec.cpy();
        osumpt += mu_calovec.sumpt();
      }
      ATH_MSG_VERBOSE("Mu cluster sumpt " << mu_calovec.sumpt());

      ATH_MSG_VERBOSE( "Misc cluster px, py, sumpt = " << opx << ", " << opy << ", " << osumpt );
      m_outputMetMomAcc.addParticle(metSoftClus.value(),opx,opy,osumpt);
      ATH_MSG_VERBOSE("Final soft cluster mpx " << m_outputMetMomAcc.mpx(metSoftClus.value())
                      << ", mpy " << m_outputMetMomAcc.mpy(metSoftClus.value())
                      << " sumet " << m_outputMetMomAcc.sumet(metSoftClus.value()));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode ColumnarMETMaker::rebuildTrackMET(xAOD::MissingET* metJet,
                                       const xAOD::JetContainer* jets,
                                       xAOD::MissingETAssociationHelper& helper,
                                       xAOD::MissingET* metSoftTrk,
                                       const xAOD::MissingET* coreSoftTrk,
                                       bool doJetJVT) const {
    if (!metJet) {
      ATH_MSG_ERROR("No MET object provided for track MET rebuilding");
      return StatusCode::FAILURE;
    }
    if (!metSoftTrk) {
      ATH_MSG_ERROR("No MET object provided for soft track MET rebuilding");
      return StatusCode::FAILURE;
    }
    if (!jets) {
      ATH_MSG_ERROR("No jet container provided for track MET rebuilding");
      return StatusCode::FAILURE;
    }

    columnar::MutableMetRange metCont (*static_cast<MissingETContainer*>(metJet->container()));
    return rebuildJetMET(columnar::MutableMetId(*metJet),metCont,columnar::JetRange(*jets),m_assocAcc(helper),std::nullopt,nullptr,columnar::MutableMetId(*metSoftTrk),coreSoftTrk,doJetJVT,true);
  }

  StatusCode ColumnarMETMaker::rebuildTrackMET(columnar::MutableMetId metJet,
                                       columnar::MutableMetRange metCont,
                                       columnar::JetRange jets,
                                       columnar::MetAssociationHelper<> helper,
                                       columnar::MutableMetId metSoftTrk,
                                       columnar::Met1Id coreSoftTrk,
                                       bool doJetJVT) const {
    return rebuildJetMET(metJet,metCont,jets,helper,std::nullopt,nullptr,metSoftTrk,coreSoftTrk,doJetJVT,true);
  }

  // **** Remove objects and any overlaps from MET calculation ****
  StatusCode ColumnarMETMaker::markInvisible(const xAOD::IParticleContainer* collection,
                                     xAOD::MissingETAssociationHelper& helper,
                                     xAOD::MissingETContainer* metCont) const
  {
    if (!collection) {
      ATH_MSG_ERROR("No object container provided for marking invisible");
      return StatusCode::FAILURE;
    }
    if (!metCont) {
      ATH_MSG_ERROR("No MET container provided for marking invisible");
      return StatusCode::FAILURE;
    }

    return markInvisible(columnar::ParticleRange(*collection),m_assocAcc(helper),columnar::MutableMetRange(*metCont));
  }

  StatusCode ColumnarMETMaker::markInvisible(columnar::ParticleRange collection,
                                     columnar::MetAssociationHelper<> helper,
                                     columnar::MutableMetRange metCont) const
  {
    columnar::MutableMetId met = m_outputMetMapAcc.fillMET (metCont, "Invisibles", invisSource);
    return rebuildMET(met,collection,helper,MissingETBase::UsageHandler::PhysicsObject);
  }

  bool ColumnarMETMaker::acceptTrack(const xAOD::TrackParticle* trk, const xAOD::Vertex* vx) const
  {
    return static_cast<bool>(m_trkseltool->accept( *trk, vx ));
  }

  const xAOD::Vertex* ColumnarMETMaker::getPV() const {

    SG::ReadHandle<xAOD::VertexContainer> h_PV(m_PVkey);

    if(!h_PV.isValid()) {
      ATH_MSG_WARNING("Unable to retrieve primary vertex container PrimaryVertices");
      return nullptr;
    }
    ATH_MSG_DEBUG("Successfully retrieved primary vertex container");
    if(h_PV->empty()) ATH_MSG_WARNING("Event has no primary vertices!");
    for(const xAOD::Vertex* vx : *h_PV) {
      if(vx->vertexType()==xAOD::VxType::PriVtx) return vx;
    }
    return nullptr;
  }



  void ColumnarMETMaker::callEvents (columnar::EventContextRange events) const
  {
    for (columnar::EventContextId event : events)
    {
      auto met = m_outputMetHandle (event);
      auto metcore = m_inputMetHandle (event);
      auto metassoc = m_metAssocHandle (event);
      if (m_columnarOperation.value() == 0u)
      {
        auto particles = m_particlesHandle (event);
        if (rebuildMET (m_columnarTermName.value(), xAODType::ObjectType(m_columnarParticleType.value()), met, particles, m_assocAcc(metassoc), MissingETBase::UsageHandler::PhysicsObject).isFailure())
          throw std::runtime_error ("Failed to rebuild MET");
      } else if (m_columnarOperation.value() == 1u)
      {
        auto jets = m_jetsHandle (event);
        if (rebuildJetMET (m_columnarJetKey.value(), m_columnarSoftClusKey.value(), met, jets, metcore, m_assocAcc(metassoc), m_columnarDoJetJVT.value()).isFailure())
          throw std::runtime_error ("Failed to rebuild jet MET");
      } else if (m_columnarOperation.value() == 2u)
      {
        auto jets = m_jetsHandle (event);
        if (rebuildTrackMET (m_columnarJetKey.value(), m_columnarSoftClusKey.value(), met, jets, metcore, m_assocAcc(metassoc), m_columnarDoJetJVT.value()).isFailure())
          throw std::runtime_error ("Failed to rebuild track MET");
      } else
      {
        throw std::runtime_error ("Unknown columnar operation");
      }
    }
  }

} //> end namespace met
