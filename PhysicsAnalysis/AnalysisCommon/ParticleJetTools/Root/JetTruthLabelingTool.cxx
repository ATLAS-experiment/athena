/* 
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/JetTruthLabelingTool.h"

#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgTools/CurrentContext.h"
#include "AthContainers/ConstAccessor.h"
#include "TruthUtils/HepMCHelpers.h"

JetTruthLabelingTool::JetTruthLabelingTool(const std::string& name) :
  asg::AsgTool(name)
{}

JetTruthLabelingTool::TruthLabelConfiguration
JetTruthLabelingTool::parseLabel(const std::string& name) {

    if (name == "R10TruthLabel_R21Precision_2022v1")
        return TruthLabelConfiguration::R21Precision_2022v1;

    if (name == "R10TruthLabel_R22v1")
        return TruthLabelConfiguration::R10TruthLabel_R22v1;

    if (name == "R10WZTruthLabel_R22v1")
        return TruthLabelConfiguration::R10WZTruthLabel_R22v1;

    if (name == "R4TruthLabel")
        return TruthLabelConfiguration::R4TruthLabel;

    if (name == "R4TruthDressedWZLabel")
        return TruthLabelConfiguration::R4TruthDressedWZLabel;

    if (name == "R4InTimeTruthLabel")
        return TruthLabelConfiguration::R4InTimeTruthLabel;

    if (name == "R4OutOfTimeTruthLabel")
        return TruthLabelConfiguration::R4OutOfTimeTruthLabel;

    return TruthLabelConfiguration::Unknown;
}


StatusCode JetTruthLabelingTool::initialize(){

  ATH_MSG_INFO("Initializing " << name());

  m_truthLabelConfig = parseLabel(m_truthLabelName);

  /// Check if TruthLabelName is supported. If not, give an error and return FAILURE
  if(m_truthLabelConfig == TruthLabelConfiguration::Unknown) {
    ATH_MSG_ERROR("TruthLabelName " << m_truthLabelName << " is not supported. Exiting...");
    return StatusCode::FAILURE;
  }

  /// Ghost Association values
  m_useGhostJetMatch = false;
  m_recoGhostFrac = 0.75;

  switch(m_truthLabelConfig) {
  /// Hard-code some values for R10TruthLabel_R21Precision_2022v1 and R10TruthLabel_R22v1                                                                                                     
  case TruthLabelConfiguration::R21Precision_2022v1:
  case TruthLabelConfiguration::R10TruthLabel_R22v1:
    m_truthJetCollectionKey="AntiKt10TruthJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    if ( m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1) {
      m_getTruthGroomedJetValues = true;
      m_truthGroomedJetCollectionKey="AntiKt10TruthSoftDropBeta100Zcut10Jets";
    } else {
      m_getTruthGroomedJetValues = false;
    }
    break;
  /// Hard-code some values for R10WZTruthLabel_R22v1
  case TruthLabelConfiguration::R10WZTruthLabel_R22v1:
    m_truthJetCollectionKey="AntiKt10TruthDressedWZJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mLowZ = 50.0;
    m_getTruthGroomedJetValues = true;
    m_truthGroomedJetCollectionKey="AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets";
    break;

  /// Hard code for small-R jets
  case TruthLabelConfiguration::R4TruthLabel:
  case TruthLabelConfiguration::R4TruthDressedWZLabel:
  case TruthLabelConfiguration::R4InTimeTruthLabel:
  case TruthLabelConfiguration::R4OutOfTimeTruthLabel:
    switch (m_truthLabelConfig) {
    case TruthLabelConfiguration::R4TruthLabel:
      m_truthJetCollectionKey = "AntiKt4TruthJets";
      break;
    case TruthLabelConfiguration::R4TruthDressedWZLabel:
      m_truthJetCollectionKey = "AntiKt4TruthDressedWZJets";
      break;
    case TruthLabelConfiguration::R4InTimeTruthLabel:
      m_truthJetCollectionKey = "InTimeAntiKt4TruthJets";
      break;
    case TruthLabelConfiguration::R4OutOfTimeTruthLabel:
      m_truthJetCollectionKey = "OutOfTimeAntiKt4TruthJets";
      break;
    default: // Cannot hit anything else
      break;
    }
    m_matchUngroomedParent = false;
    m_dRTruthJet = 0.3;
    m_getTruthGroomedJetValues = false;
    m_useGhostJetMatch = false;
    m_doLargeRLabels = false;
    break;
  default:
    ATH_MSG_ERROR(" Unhandled TruthLabelName " << m_truthLabelName << "! Exiting...");
    return StatusCode::FAILURE;
  }

  if(m_forceDeltaRMatch) {
    m_useGhostJetMatch = false;
  }

  print();

  m_label_truthKey   = m_truthJetCollectionKey.key() + "." + m_truthLabelName;
  m_NB_truthKey      = m_truthJetCollectionKey.key() + "." + m_truthLabelName + "_NB";
  m_split12_truthKey = m_truthJetCollectionKey.key() + ".Split12";
  m_split23_truthKey = m_truthJetCollectionKey.key() + ".Split23";

  if(!m_isTruthJetCol){
    m_label_recoKey  = m_jetContainerName + "." + m_truthLabelName;
    m_NB_recoKey     = m_jetContainerName + "." + m_truthLabelName + "_NB";
    m_truthSplit12_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit12";
    m_truthSplit23_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit23";

    m_matchedTruthJet_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_Jet";
    m_matchedTruthJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_m";
    m_matchedTruthJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_pt";
    m_matchedTruthJetRapidity_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_rapidity";
    m_matchedTruthJetPhi_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_phi";
    m_matchedTruthJetDR_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_dR";

    if (!m_doLargeRLabels) { //Simple naming for small R
        m_matchedTruthJet_recoKey = m_jetContainerName + "." + "TruthMatch_Jet";
        m_matchedTruthJetMass_recoKey = m_jetContainerName + "." + "TruthMatch_m";
        m_matchedTruthJetPt_recoKey = m_jetContainerName + "." + "TruthMatch_pt";
        m_matchedTruthJetRapidity_recoKey = m_jetContainerName + "." + "TruthMatch_rapidity";
        m_matchedTruthJetPhi_recoKey = m_jetContainerName + "." + "TruthMatch_phi";
        m_matchedTruthJetDR_recoKey = m_jetContainerName + "." + "TruthMatch_dR";
        m_matchedPileupTag_recoKey = m_jetContainerName + "." + "TruthMatch_PileupLabel";
        m_sumPtMatchedHSJets_recoKey = m_jetContainerName + "." + "TruthMatch_HSSumPt";
        m_sumPtMatchedOOTJets_recoKey = m_jetContainerName + "." + "TruthMatch_OOTSumPt";
        m_sumPtMatchedITJets_recoKey = m_jetContainerName + "." + "TruthMatch_ITSumPt";
    }

    m_matchedTruthGroomedJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_GroomedJetMass";
    m_matchedTruthGroomedJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthMatch_GroomedJetPt";
  }

  ATH_CHECK(m_evtInfoKey.initialize());
  ATH_CHECK(m_truthJetCollectionKey.initialize());
  ATH_CHECK(m_truthGroomedJetCollectionKey.initialize(m_getTruthGroomedJetValues));

  ATH_CHECK(m_ITJetName.initialize(!m_doLargeRLabels));
  ATH_CHECK(m_OOTJetName.initialize(!m_doLargeRLabels));
  
  ATH_CHECK(m_label_truthKey.initialize());
  ATH_CHECK(m_NB_truthKey.initialize());
  
  ATH_CHECK(m_split12_truthKey.initialize(m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1));
  ATH_CHECK(m_split23_truthKey.initialize(m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1));

  ATH_CHECK(m_label_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_NB_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_truthSplit12_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) ));
  ATH_CHECK(m_truthSplit23_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1)));
  
  ATH_CHECK(m_matchedTruthJet_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetMass_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetPt_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetRapidity_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetPhi_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetDR_recoKey.initialize(!m_isTruthJetCol));

  ATH_CHECK(m_matchedPileupTag_recoKey.initialize(!m_isTruthJetCol && !m_doLargeRLabels));
  ATH_CHECK(m_sumPtMatchedHSJets_recoKey.initialize(!m_isTruthJetCol && !m_doLargeRLabels));
  ATH_CHECK(m_sumPtMatchedOOTJets_recoKey.initialize(!m_isTruthJetCol && !m_doLargeRLabels));
  ATH_CHECK(m_sumPtMatchedITJets_recoKey.initialize(!m_isTruthJetCol && !m_doLargeRLabels));

  ATH_CHECK(m_matchedTruthGroomedJetMass_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));
  ATH_CHECK(m_matchedTruthGroomedJetPt_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));

  return StatusCode::SUCCESS;
}

void JetTruthLabelingTool::print() const {
  ATH_MSG_INFO("Parameters for " << name());

  ATH_MSG_INFO("xAOD information:");
  ATH_MSG_INFO("TruthLabelName:               " << m_truthLabelName);
  ATH_MSG_INFO("TruthJetCollectionName:         " << m_truthJetCollectionKey.key());
  ATH_MSG_INFO("dRTruthJet:    " << std::to_string(m_dRTruthJet));

  if(m_getTruthGroomedJetValues) {
    ATH_MSG_INFO("truthGroomedJetCollectionName: " << m_truthGroomedJetCollectionKey.key());
  }
}


JetTruthLabelingTool::DecorHandles::DecorHandles
  (const JetTruthLabelingTool& tool, const EventContext& ctx)
{
  auto maybeInit = [&] (auto& h,
                        const SG::WriteDecorHandleKey<xAOD::JetContainer>& k)
  {
    if (!k.key().empty()) h.emplace (k, ctx);
  };
  maybeInit (labelHandle,     tool.m_label_truthKey);
  maybeInit (nbHandle,        tool.m_NB_truthKey);
  maybeInit (labelRecoHandle, tool.m_label_recoKey);
  maybeInit (nbRecoHandle,    tool.m_NB_recoKey);
  maybeInit (split23Handle,   tool.m_truthSplit23_recoKey);
  maybeInit (split12Handle,   tool.m_truthSplit12_recoKey);

  maybeInit (matchedTruthJetHandle, tool.m_matchedTruthJet_recoKey);
  maybeInit (matchedTruthJetMassHandle, tool.m_matchedTruthJetMass_recoKey);
  maybeInit (matchedTruthJetPtHandle,   tool.m_matchedTruthJetPt_recoKey);
  maybeInit (matchedTruthJetRapidityHandle, tool.m_matchedTruthJetRapidity_recoKey);
  maybeInit (matchedTruthJetPhiHandle, tool.m_matchedTruthJetPhi_recoKey);
  maybeInit (matchedTruthJetDRHandle, tool.m_matchedTruthJetDR_recoKey);

  maybeInit (matchedPileupTagHandle, tool.m_matchedPileupTag_recoKey);
  maybeInit (sumPtMatchedHSJetsHandle, tool.m_sumPtMatchedHSJets_recoKey);
  maybeInit (sumPtMatchedOOTJetsHandle, tool.m_sumPtMatchedOOTJets_recoKey);
  maybeInit (sumPtMatchedITJetsHandle, tool.m_sumPtMatchedITJets_recoKey);

  maybeInit (matchedTruthGroomedMassHandle, tool.m_matchedTruthGroomedJetMass_recoKey);
  maybeInit (matchedTruthGroomedPtHandle,   tool.m_matchedTruthGroomedJetPt_recoKey);
}

int JetTruthLabelingTool::getTruthJetLabelGA( DecorHandles& dh,
                                              const xAOD::Jet &jet,
                                              const EventContext& ctx ) const
{
  /// Booleans to check ghost-associated heavy particles
  bool matchW = false;
  bool matchZ = false;
  bool matchH = false;
  bool matchTop = false;

  /// Find ghost associated W bosons
  int nMatchW = getNGhostParticles( jet, "GhostWBosons" );

  if ( nMatchW ) {
    matchW = true;
  }

  /// Find ghost associated Z bosons
  int nMatchZ = getNGhostParticles( jet, "GhostZBosons" );
  
  if ( nMatchZ ) {
    matchZ = true;
  }

  /// Find ghost associated H bosons
  int nMatchH = getNGhostParticles( jet, "GhostHBosons" );
  
  if ( nMatchH ) {
    matchH = true;
  }

  /// Find ghost associated top quarks
  int nMatchTop = getNGhostParticles( jet, "GhostTQuarksFinal" );

  if ( nMatchTop ) {
    matchTop = true;
  }
 
  return getLabel( dh, jet, matchH, matchW, matchZ, matchTop, ctx );

}

StatusCode JetTruthLabelingTool::decorate(const xAOD::JetContainer& jets) const {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  DecorHandles dh (*this, ctx);

  /// Apply label to truth jet collections
  if(m_isTruthJetCol) {
    return labelTruthJets(dh, jets, ctx);
  }

  else if (!m_isTruthJetCol && !m_doLargeRLabels) {
      return labelRecoJets(dh, jets, ctx);
  }

  /// Copy label to matched reco jets
  else {
    ATH_CHECK( labelTruthJets(dh, ctx) );
    return labelRecoJets(dh, jets, ctx);
  }

  return StatusCode::SUCCESS;
}

StatusCode JetTruthLabelingTool::labelRecoJets(DecorHandles& dh,
                                               const xAOD::JetContainer& jets,
                                               const EventContext& ctx) const {
    
  SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetCollectionKey, ctx);

  bool usePileupJets = true;

  SG::ReadHandle<xAOD::JetContainer> inTimeTruthJets;
  SG::ReadHandle<xAOD::JetContainer> outOfTimeTruthJets;
  if (!m_doLargeRLabels) {
      inTimeTruthJets = SG::makeHandle(m_ITJetName, ctx);
      outOfTimeTruthJets = SG::makeHandle(m_OOTJetName, ctx);

      if (!inTimeTruthJets.isValid() || !outOfTimeTruthJets.isValid()) {
          ATH_MSG_WARNING("Pileup jet collections unavailabel for " << m_truthJetCollectionKey << ". Running without pileup tagger.");
          usePileupJets = false;
      }
  }
  
  SG::ReadHandle<xAOD::JetContainer> truthGroomedJets;
  if ( m_getTruthGroomedJetValues ) {
    truthGroomedJets = SG::makeHandle(m_truthGroomedJetCollectionKey, ctx);
  }

  const SG::Accessor<int> nbAcc (m_truthLabelName + "_NB");
  static const SG::ConstAccessor< ElementLink< xAOD::JetContainer > > ParentAcc ("Parent");

  for(const xAOD::Jet *jet : jets) {

    /// Get parent ungroomed reco jet for matching
    const xAOD::Jet* parent = nullptr;
    if ( m_matchUngroomedParent ){
      ElementLink<xAOD::JetContainer> element_link = ParentAcc (*jet);
      if ( element_link.isValid() ) {
        parent = *element_link;
      }
      else {
        ATH_MSG_ERROR("Unable to get a link to the parent jet! Returning a NULL pointer."); 
        return StatusCode::FAILURE;
      }
    }
    /// Find matched truth jet
    float dRmin = 9999;
    float oot_dRmin = 9999;
    float it_dRmin = 9999;
    float ghostFracNominal = 9;
    const xAOD::Jet* matchTruthJet = nullptr;
    const xAOD::Jet* oot_matchTruthJet = nullptr;
    const xAOD::Jet* it_matchTruthJet = nullptr;
    /// Groomed Jet matching variables (Large R only)
    float dRminGroomed = 9999;
    const xAOD::Jet* matchTruthGroomedJet = nullptr;
    /// Pileup tagging variables
    float sumPtMatchedHSJets = 0;
    float sumPtMatchedOOTJets = 0;
    float sumPtMatchedITJets = 0;
    float bestHSpT = -9999;
    float bestOOTpT = -9999;
    float bestITpT = -9999;
    float bestHSpTRatio = -9;
    float bestOOTpTRatio = -9;
    float bestITpTRatio = -9;
    int nMatchedHSJets = 0;
    int nMatchedOOTJets = 0;
    int nMatchedITJets = 0;
    int pileupTag = SmallRJetPileupLabel::enumToInt(SmallRJetPileupLabel::Unknown);
    //coverity[UNNECESSARY_STRING_COPY:FALSE]
    static const SG::ConstAccessor<float> accGhostTruthPt("GhostTruthPt");

    // Ensure that the reco jet has at least one constituent
    // (and thus a well-defined four-vector)
    if (jet->numConstituents() > 0) {
        for (const xAOD::Jet* truthJet : *truthJets) { // Truth jet loop
            // Calculate DR
            float dR = jet->p4().DeltaR(truthJet->p4(),true);
            // If parent jet has been retrieved, calculate dR w.r.t. it instead
            if (parent) dR = parent->p4().DeltaR(truthJet->p4(),true);
            // Calculate GF
            if (m_useGhostJetMatch) { // GA matching. Upper bound applied for completeness, but realistically not needed.
                // Access ghostTruthPt now so it is available for pileup jets
                float ghostTruthPt = accGhostTruthPt(*jet);
                float ghostPtFraction = (ghostTruthPt / (truthJet->pt()));
                if ((ghostPtFraction >= m_recoGhostFrac) && (ghostPtFraction <= (2 - m_recoGhostFrac))) { // All matches
                    nMatchedHSJets += 1;
                    sumPtMatchedHSJets += truthJet->pt();
                    if (std::abs(1 - ghostPtFraction) < std::abs(1 - ghostFracNominal)) { // Best match
                        matchTruthJet = truthJet;
                        ghostFracNominal = ghostPtFraction;
                    }
                }
            }
            else { // If m_dRTruthJet < 0, the closest truth jet is used as matched jet. Otherwise, only match if dR < m_dRTruthJet
                if (m_dRTruthJet < 0 || dR < m_dRTruthJet) { // All matches
                    nMatchedHSJets += 1;
                    sumPtMatchedHSJets += truthJet->pt();
                    if (dR < dRmin) { // Best match 
                        dRmin = dR;
                        matchTruthJet = truthJet;
                    }
                }
            }
        }
        if (!m_doLargeRLabels && usePileupJets) {
            for (const xAOD::Jet* ootJet : *outOfTimeTruthJets) { // Out of time pileup truth jet loop
                float dR = jet->p4().DeltaR(ootJet->p4(), true);
                if (m_useGhostJetMatch) {
                    float ghostTruthPt = accGhostTruthPt(*jet);
                    float ghostPtFraction = (ghostTruthPt / (ootJet->pt()));
                    if ((ghostPtFraction >= m_recoGhostFrac) && (ghostPtFraction <= (2 - m_recoGhostFrac))) { // All matches
                        nMatchedOOTJets += 1;
                        sumPtMatchedOOTJets += ootJet->pt();
                        if (std::abs(1 - ghostPtFraction) < std::abs(1 - ghostFracNominal)) { // Best match
                            oot_matchTruthJet = ootJet;
                        }
                    }
                }
                else {
                    if (dR < m_dRTruthJet) { // All matches
                        nMatchedOOTJets += 1;
                        sumPtMatchedOOTJets += ootJet->pt();
                        if (dR < oot_dRmin) { // Best match
                            oot_dRmin = dR;
                            oot_matchTruthJet = ootJet;
                        }
                    }
                }
            }
            for (const xAOD::Jet* itJet : *inTimeTruthJets) { // In time pileup truth jet loop
                float dR = jet->p4().DeltaR(itJet->p4(), true);
                if (m_useGhostJetMatch) {
                    float ghostTruthPt = accGhostTruthPt(*jet);
                    float ghostPtFraction = (ghostTruthPt / (itJet->pt()));
                    if ((ghostPtFraction >= m_recoGhostFrac) && (ghostPtFraction <= (2 - m_recoGhostFrac))) { // All matches
                        nMatchedITJets += 1;
                        sumPtMatchedITJets += itJet->pt();
                        if (std::abs(1 - ghostPtFraction) < std::abs(1 - ghostFracNominal)) { // Best match
                            it_matchTruthJet = itJet;
                        }
                    }
                }
                else {
                    if (dR < m_dRTruthJet) { // All matches
                        nMatchedITJets += 1;
                        sumPtMatchedITJets += itJet->pt();
                        if (dR < it_dRmin) { // Best match
                            it_dRmin = dR;
                            it_matchTruthJet = itJet;
                        }
                    }
                }
            }

            //#######################################################################################
            //# PILEUP TAGGER KEY                                                                   #
            //# 0: Hard scatter ------------- Best truth jet match has >90% of the total matched pT #
            //# 1: Mixed hard scatter-------- Match with multiple truth jets, none >90% (no PU)     #
            //# 2: Hard scatter with pileup - Best truth jet <90%, also has pileup matches          #
            //# 3: In time pileup---------- - Best it jet match has > 90 % of the total matched pT  #
            //# 4: Mixed pileup ------------- Match with multiple pileup jets, none >90% (no HS)    #
            //# 5: Out of time pileup------ - Best oot jet match has > 90 % of the total matched pT #
            //# 6: Other/unknown ------------ Fails all above cases                                 #
            //#######################################################################################
            
            //NOTE: HS has 5 GeV pT threshold, 10 GeV and 15 GeV for IT and OOT pileup.

            int nMatchedPUJets = nMatchedOOTJets + nMatchedITJets;

            float totalMatchedpT = sumPtMatchedHSJets + sumPtMatchedOOTJets + sumPtMatchedITJets;

            if (matchTruthJet) {
                bestHSpT = matchTruthJet->pt();
                bestHSpTRatio = bestHSpT / totalMatchedpT;
            }
            if (oot_matchTruthJet) {
                bestOOTpT = oot_matchTruthJet->pt();
                bestOOTpTRatio = bestOOTpT / totalMatchedpT;
            }
            if (it_matchTruthJet) {
                bestITpT = it_matchTruthJet->pt();
                bestITpTRatio = bestITpT / totalMatchedpT;
            }

            if (bestHSpTRatio >= 0.9) { //HS
                pileupTag = SmallRJetPileupLabel::HS;
            }
            else if (bestITpTRatio >= 0.9) { //IT
                pileupTag = SmallRJetPileupLabel::ITPU;
            }
            else if (bestOOTpTRatio >= 0.9) { //OOT
                pileupTag = SmallRJetPileupLabel::OOTPU;
            }
            else if (nMatchedPUJets == 0 && nMatchedHSJets > 1) { //Mixed HS
                pileupTag = SmallRJetPileupLabel::MixHS;
            }
            else if (nMatchedPUJets > 0 && nMatchedHSJets > 0) { //HS with pileup
                pileupTag = SmallRJetPileupLabel::HSPU;
            }
            else if (nMatchedPUJets > 1) {//Mixed PU
                pileupTag = SmallRJetPileupLabel::MixPU;
            }
            else { //No match
                pileupTag = SmallRJetPileupLabel::Unknown;
            }

        }
        
    }
        
    int label = LargeRJetTruthLabel::enumToInt( LargeRJetTruthLabel::notruth );
    int truthJetNB = -1;
    float truthJetSplit12 = -9999;
    float truthJetSplit23 = -9999;

    // Defaults to null EL
    ElementLink<xAOD::JetContainer> truthJetEL{};
    float truthJetMass = -9999;
    float truthJetPt = -9999;
    float truthJetRapidity = -9;
    float truthJetPhi = -9999;
    float deltaR = -2;

    const xAOD::JetContainer* truthJetCont = truthJets.cptr();

    //If the best match is PU, reassign the truth jet to take variables from
    switch(pileupTag) {
      case SmallRJetPileupLabel::ITPU: {
        matchTruthJet = it_matchTruthJet;
        truthJetCont = inTimeTruthJets.cptr();
        break;
      }
    case SmallRJetPileupLabel::OOTPU: {
        matchTruthJet = oot_matchTruthJet;
        truthJetCont = outOfTimeTruthJets.cptr();
        break;
      }
    case SmallRJetPileupLabel::MixPU: {
        if (bestITpTRatio >= bestOOTpTRatio) {
          matchTruthJet = it_matchTruthJet;
          truthJetCont = inTimeTruthJets.cptr();
        }
        else {
          matchTruthJet = oot_matchTruthJet;
          truthJetCont = outOfTimeTruthJets.cptr();
        }
      }
    default:
      break;
    }

    if ( matchTruthJet ) {

        // Can't use the WriteDecorHandle to read --- the decoration may have
        // been added and locked by a previous algorithm.
        // Not saving Truth jet decorations for small R
        if (m_doLargeRLabels) {
            SG::ConstAccessor<int> labelAcc(dh.labelHandle->auxid());
            label = labelAcc(*matchTruthJet);
            if (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {
                SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
                if (split23Handle.isAvailable()) truthJetSplit23 = split23Handle(*matchTruthJet);
                SG::ReadDecorHandle<xAOD::JetContainer, float> split12Handle(m_split12_truthKey, ctx);
                if (split12Handle.isAvailable()) truthJetSplit12 = split12Handle(*matchTruthJet);
            }
            if (nbAcc.isAvailable(*matchTruthJet)) truthJetNB = nbAcc(*matchTruthJet);
        }
        ATH_MSG_VERBOSE("For reco jet " << jet->index() << ", matched truth jet index " << matchTruthJet->index());
        truthJetEL = ElementLink<xAOD::JetContainer>(*truthJetCont, matchTruthJet->index(), ctx);
        truthJetMass = matchTruthJet->m();
        truthJetPt = matchTruthJet->pt();
        truthJetRapidity = matchTruthJet->rapidity();
        truthJetPhi = matchTruthJet->phi();
        deltaR = dRmin;
    }

    // Save Groomed Truth Jet variables
    float truthGroomedJetMass = -9999;
    float truthGroomedJetPt = -9999;
    if ( m_getTruthGroomedJetValues ) {
      if ( matchTruthJet ) {
        for ( const xAOD::Jet* truthGroomedJet : *truthGroomedJets ) {
          ElementLink<xAOD::JetContainer> element_link = ParentAcc (*truthGroomedJet);
          if ( !element_link.isValid() ) { continue; }
          if ( matchTruthJet == *element_link ) {
            matchTruthGroomedJet = truthGroomedJet;
            break;
          }
        }
      }
      // If no matched jet found or matched jet has no corresponding groomed jet, use dR matching
      if ( !matchTruthGroomedJet && parent != nullptr) {
        for ( const xAOD::Jet* truthGroomedJet : *truthGroomedJets ) {
	        float dR = parent->p4().DeltaR(truthGroomedJet->p4(),true);
	        /// If m_dRTruthJet < 0, the closest truth jet is used as matched jet. Otherwise, only match if dR < m_dRTruthJet
	        if ( m_dRTruthJet < 0 || dR < m_dRTruthJet ) {
	          if ( dR < dRminGroomed ) {
	            dRminGroomed = dR;
	            matchTruthGroomedJet = truthGroomedJet;
	          }
	        }
        }
      }
      if ( matchTruthGroomedJet ) {
        truthGroomedJetMass = matchTruthGroomedJet->m();
        truthGroomedJetPt = matchTruthGroomedJet->pt();
      }
    }

    /// Decorate truth label
    if (m_doLargeRLabels) {
        (*dh.labelRecoHandle)(*jet) = label;
        /// Decorate additional information used for truth labeling
        if (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {
            (*dh.split23Handle)(*jet) = truthJetSplit23;
            (*dh.split12Handle)(*jet) = truthJetSplit12;
        }

        (*dh.nbRecoHandle)(*jet) = truthJetNB;
    }
    
    (*dh.matchedTruthJetHandle)(*jet) = truthJetEL;
    (*dh.matchedTruthJetMassHandle)(*jet) = truthJetMass;
    (*dh.matchedTruthJetPtHandle)(*jet) = truthJetPt;
    (*dh.matchedTruthJetRapidityHandle)(*jet) = truthJetRapidity;
    (*dh.matchedTruthJetPhiHandle)(*jet) = truthJetPhi;
    (*dh.matchedTruthJetDRHandle)(*jet) = deltaR;

    if (!m_doLargeRLabels && usePileupJets) {
        (*dh.matchedPileupTagHandle)(*jet) = pileupTag;
        (*dh.sumPtMatchedHSJetsHandle)(*jet) = sumPtMatchedHSJets;
        (*dh.sumPtMatchedOOTJetsHandle)(*jet) = sumPtMatchedOOTJets;
        (*dh.sumPtMatchedITJetsHandle)(*jet) = sumPtMatchedITJets;
    }

    if ( m_getTruthGroomedJetValues ) {
      (*dh.matchedTruthGroomedMassHandle)(*jet) = truthGroomedJetMass;
      (*dh.matchedTruthGroomedPtHandle)(*jet) = truthGroomedJetPt;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode JetTruthLabelingTool::labelTruthJets(DecorHandles& dh,
                                                const EventContext& ctx) const {

  /// Retrieve appropriate truth jet container
  SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetCollectionKey, ctx);

  /// Make sure the truth jet collection has been retrieved
  if ( !truthJets.isValid() ) {
    ATH_MSG_ERROR("No truth jet container retrieved. Please make sure you are using a supported TruthLabelName.");
    return StatusCode::FAILURE;
  }

  return labelTruthJets(dh, *truthJets, ctx);

}

StatusCode JetTruthLabelingTool::labelTruthJets( DecorHandles& dh,
                                                 const xAOD::JetContainer &truthJets,
                                                 const EventContext& ctx) const
{
  /// Make sure there is at least 1 jet in truth collection
  if ( !(truthJets.size()) ) return StatusCode::SUCCESS;

  /// Check if the truth jet collection already has labels applied
  if(dh.labelHandle->isAvailable()){
    // Beware: if we get here, the configuration is probably not MT-compatible.
    ATH_MSG_DEBUG("labelTruthJets: Truth jet collection already labelled with " << m_truthLabelName);
    return StatusCode::SUCCESS;
  }

  /// Get the EventInfo to identify Sherpa samples
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfoKey, ctx);
  if(!eventInfo.isValid()){
    ATH_MSG_ERROR("Failed to retrieve event information.");
    return StatusCode::FAILURE;
  }

  if(m_doLargeRLabels) {
    int label = LargeRJetTruthLabel::enumToInt(LargeRJetTruthLabel::notruth);
    /// Apply label to truth jet
    for ( const xAOD::Jet *jet : truthJets ) {
      ATH_MSG_DEBUG("Getting truth label using ghost-association");
      label = getTruthJetLabelGA(dh, *jet, ctx);
      (*dh.labelHandle)(*jet) = label;
    }
  }

  return StatusCode::SUCCESS;
}

float JetTruthLabelingTool::getWZSplit12Cut( float pt ) const {

  /// The functional form and parameters come from optimization studies:
  /// https://cds.cern.ch/record/2777009/files/ATL-PHYS-PUB-2021-029.pdf

  float split12 = -999.0;

  if ( m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {
    const float c0 = 55.25;
    const float c1 = -2.34e-3;

    split12 = c0 * std::exp( c1 * pt );
  }

  return split12;

}

float JetTruthLabelingTool::getTopSplit23Cut( float pt ) const {

  /// The functional form and parameters come from optimization studies:
  /// https://indico.cern.ch/event/931498/contributions/3921872/attachments/2064188/3463746/JSS_25June.pdf

  float split23 = -999.0;

  if ( m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 ||
       m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 ||
       m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {

    const float c0 = 3.3;
    const float c1 = -6.98e-4;

    split23 = std::exp( c0 + c1 * pt );

  }

  return split23;
}

int JetTruthLabelingTool::getNGhostParticles( const xAOD::Jet &jet, const std::string & collection ) const {

  int nMatchPart = 0;

  if( !jet.getAttribute<int>( collection+"Count", nMatchPart ) ){

    std::vector<const xAOD::TruthParticle*> ghostParts;
    if( !jet.getAssociatedObjects<xAOD::TruthParticle>( collection, ghostParts ) ){      
      ATH_MSG_ERROR( collection + " cannot be retrieved! Truth label definition might be wrong" );
    } 
    nMatchPart = ghostParts.size();
  }

  return nMatchPart;
}

int JetTruthLabelingTool::getLabel( DecorHandles& dh,
                                    const xAOD::Jet &jet,
                                    bool matchH,
                                    bool matchW,
                                    bool matchZ,
                                    bool matchTop,
                                    const EventContext& ctx) const {

  // store GhostBHadronsFinal count
  int nMatchB = getNGhostParticles( jet, "GhostBHadronsFinal" );
  (*dh.nbHandle)(jet) = nMatchB;

  /// Booleans for containment selections
  bool is_bb = false;
  bool is_cc = false;
  bool is_tautauEl = false;
  bool is_tautauMu = false;
  bool is_tautauHad = false;
  bool isTop = false;
  bool isW = false;
  bool isZ = false;

  // Use R21Precision_2022v1 definition
  if ( m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1) {
    SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, float> split12Handle(m_split12_truthKey, ctx);
    is_bb = ( nMatchB > 1 );
    isTop = ( matchTop && matchW && nMatchB > 0 && jet.m() / 1000. > m_mLowTop && split23Handle(jet) / 1000. > getTopSplit23Cut( jet.pt() / 1000. ) );
    isW = matchW && nMatchB == 0 && jet.m() / 1000. > m_mLowW && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
    isZ = matchZ && jet.m() / 1000. > m_mLowZ && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
  }

  // Use R10TruthLabel_R22v1 definition
  if ( m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {
    // get extended ghost associated truth label
    int extended_GA_label = -1;
    if (not jet.getAttribute("HadronGhostExtendedTruthLabelID", extended_GA_label)) {
      ATH_MSG_ERROR( "HadronGhostExtendedTruthLabelID not available for " + m_truthJetCollectionKey.key() );
    }

    SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, float> split12Handle(m_split12_truthKey, ctx);
    is_bb = ( extended_GA_label == 55 );
    is_cc = ( extended_GA_label == 44 );
    is_tautauEl = ( extended_GA_label == 151511 );
    is_tautauMu = ( extended_GA_label == 151513 );
    is_tautauHad = ( extended_GA_label == 1515 );
    isTop = ( matchTop && matchW && nMatchB > 0 && jet.m() / 1000. > m_mLowTop && split23Handle(jet) / 1000. > getTopSplit23Cut( jet.pt() / 1000. ) );
    isW = matchW && nMatchB == 0 && jet.m() / 1000. > m_mLowW && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
    isZ = matchZ &&  jet.m() / 1000. > m_mLowZ && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
  }


  /// This method can be expanded to include custom label priorities

  /* The default priority of labels is:
   * 1) Hbb/cc/tautau
   * 2) Contained top
   * 3) Contained W
   * 4) Contained Zbb/cc/qq/tautau
   * 5) Uncontained top
   * 6) Uncontained V
   */

  /// If it isn't matched to any heavy particles, is it QCD
  if( !(matchTop || matchW || matchZ || matchH) ) {
    return LargeRJetTruthLabel::qcd;
  }

  // Higgs
  if ( matchH ) {
    /// Contained H->bb
    if ( is_bb ) return LargeRJetTruthLabel::Hbb;
    /// Contained H->cc
    if ( is_cc ) return LargeRJetTruthLabel::Hcc;
    /// Contained H->tautau
    if ( is_tautauEl ) return LargeRJetTruthLabel::HtautauEl;
    if ( is_tautauMu ) return LargeRJetTruthLabel::HtautauMu;
    if ( is_tautauHad ) return LargeRJetTruthLabel::HtautauHad;
    /// Other from H
    return LargeRJetTruthLabel::other_From_H;
  }

  /// Contained top
  if ( isTop ) return LargeRJetTruthLabel::tqqb;

  if ( isW ) {
    /// Contained W from a top
    if ( matchTop ) return LargeRJetTruthLabel::Wqq_From_t;
    /// Contained W not from a top
    return LargeRJetTruthLabel::Wqq;
  }

  if ( matchZ ) {
    /// Contained Z->bb
    if ( is_bb ) return LargeRJetTruthLabel::Zbb;
    /// Contained Z->cc
    if ( is_cc ) return LargeRJetTruthLabel::Zcc;
    /// Contained Z->tautau
    if ( is_tautauEl ) return LargeRJetTruthLabel::ZtautauEl;
    if ( is_tautauMu ) return LargeRJetTruthLabel::ZtautauMu;
    if ( is_tautauHad ) return LargeRJetTruthLabel::ZtautauHad;
  }
  if ( isZ ) {
    /// Contained Z->qq
    return LargeRJetTruthLabel::Zqq;
  }

  /// Uncontained top
  if ( matchTop ) return LargeRJetTruthLabel::other_From_t;

  /// Uncontained V
  return LargeRJetTruthLabel::other_From_V;

}
