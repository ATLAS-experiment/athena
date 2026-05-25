/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

    return TruthLabelConfiguration::Unknown;
}


StatusCode JetTruthLabelingTool::initialize(){

  ATH_MSG_INFO("Initializing " << name());

  m_truthLabelConfig = parseLabel(m_truthLabelName);

  /// Ghost Association values
  m_useGhostJetMatch = false;
  m_recoGhostFrac = 0.75;

  /// Hard-code some values for R10TruthLabel_R21Precision_2022v1 and R10TruthLabel_R22v1                                                                                                     
  if(m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 or m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1) {
    m_truthJetCollectionName="AntiKt10TruthJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_useWZMassHigh = false;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mLowZ = 50.0;
    if ( m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1) {
      m_getTruthGroomedJetValues = true;
      m_truthGroomedJetCollectionName = "AntiKt10TruthSoftDropBeta100Zcut10Jets";
    } else {
      m_getTruthGroomedJetValues = false;
    }
  }
  /// Hard-code some values for R10WZTruthLabel_R22v1
  else if( m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1 ) {
    m_truthJetCollectionName="AntiKt10TruthDressedWZJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_useWZMassHigh = false;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mLowZ = 50.0;
    m_getTruthGroomedJetValues = true;
    m_truthGroomedJetCollectionName = "AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets";
  } 

  /// Hard code for small-R jets
  else if (m_truthLabelConfig == TruthLabelConfiguration::R4TruthLabel ) {
      m_truthJetCollectionName = "AntiKt4TruthJets";
      // m_truthJetCollectionName = "AntiKt4TruthDressedWZJets";
      // Dressed option not working as of 2026-04-17, using regular.
      m_matchUngroomedParent = false;
      m_dRTruthJet = 0.3;
      m_useWZMassHigh = true;
      m_getTruthGroomedJetValues = false;
      m_useGhostJetMatch = true;
  }

  print();

  /// Check if TruthLabelName is supported. If not, give an error and return FAILURE
  
  bool isSupportedLabel = false;
  isSupportedLabel = isSupportedLabel || (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1);
  isSupportedLabel = isSupportedLabel || (m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1);
  isSupportedLabel = isSupportedLabel || (m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1);
  isSupportedLabel = isSupportedLabel || (m_truthLabelConfig == TruthLabelConfiguration::R4TruthLabel);

  if(!isSupportedLabel) {
    ATH_MSG_ERROR("TruthLabelName " << m_truthLabelName << " is not supported. Exiting...");
    return StatusCode::FAILURE;
  }

  m_label_truthKey   = m_truthJetCollectionName.key() + "." + m_truthLabelName;
  m_NB_truthKey      = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_NB";
  m_split12_truthKey = m_truthJetCollectionName.key() + ".Split12";
  m_split23_truthKey = m_truthJetCollectionName.key() + ".Split23";

  if(!m_isTruthJetCol){
    m_label_recoKey  = m_jetContainerName + "." + m_truthLabelName;
    m_NB_recoKey     = m_jetContainerName + "." + m_truthLabelName + "_NB";
    m_truthSplit12_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit12";
    m_truthSplit23_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit23";

    m_matchedTruthJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetMass";
    m_matchedTruthJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetPt";
    m_matchedTruthJetEta_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetEta";
    m_matchedTruthJetPhi_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetPhi";
    m_matchedTruthJetDR_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetDR";
    m_matchedTruthJetGF_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthJetGF";

    m_matchedTruthGroomedJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthGroomedJetMass";
    m_matchedTruthGroomedJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_MatchedTruthGroomedJetPt";
  }

  ATH_CHECK(m_evtInfoKey.initialize());
  ATH_CHECK(m_truthJetCollectionName.initialize());
  ATH_CHECK(m_truthGroomedJetCollectionName.initialize(m_getTruthGroomedJetValues));

  ATH_CHECK(m_label_truthKey.initialize());
  ATH_CHECK(m_NB_truthKey.initialize());
  
  ATH_CHECK(m_split12_truthKey.initialize(m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1));
  ATH_CHECK(m_split23_truthKey.initialize(m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1));

  ATH_CHECK(m_label_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_NB_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_truthSplit12_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) ));
  ATH_CHECK(m_truthSplit23_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1)));
  
  ATH_CHECK(m_matchedTruthJetMass_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetPt_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetEta_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetPhi_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetDR_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_matchedTruthJetGF_recoKey.initialize(!m_isTruthJetCol));

  ATH_CHECK(m_matchedTruthGroomedJetMass_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));
  ATH_CHECK(m_matchedTruthGroomedJetPt_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));

  return StatusCode::SUCCESS;
}

void JetTruthLabelingTool::print() const {
  ATH_MSG_INFO("Parameters for " << name());

  ATH_MSG_INFO("xAOD information:");
  ATH_MSG_INFO("TruthLabelName:               " << m_truthLabelName);
  ATH_MSG_INFO("TruthJetCollectionName:         " << m_truthJetCollectionName.key());
  ATH_MSG_INFO("dRTruthJet:    " << std::to_string(m_dRTruthJet));

  if(m_truthLabelName != "R4TruthLabel"){
    ATH_MSG_INFO("mLowTop:       " << std::to_string(m_mLowTop));
    ATH_MSG_INFO("mLowW:         " << std::to_string(m_mLowW));
    if(m_useWZMassHigh)
      ATH_MSG_INFO("mHighW:        " << std::to_string(m_mHighW));
    ATH_MSG_INFO("mLowZ:         " << std::to_string(m_mLowZ));
    if(m_useWZMassHigh)
      ATH_MSG_INFO("mHighZ:        " << std::to_string(m_mHighZ));
  }

  if(m_getTruthGroomedJetValues) {
    ATH_MSG_INFO("truthGroomedJetCollectionName: " << m_truthGroomedJetCollectionName.key());
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

  maybeInit (matchedTruthJetMassHandle, tool.m_matchedTruthJetMass_recoKey);
  maybeInit (matchedTruthJetPtHandle,   tool.m_matchedTruthJetPt_recoKey);
  maybeInit (matchedTruthJetEtaHandle, tool.m_matchedTruthJetEta_recoKey);
  maybeInit (matchedTruthJetPhiHandle, tool.m_matchedTruthJetPhi_recoKey);
  maybeInit (matchedTruthJetDRHandle, tool.m_matchedTruthJetDR_recoKey);
  maybeInit (matchedTruthJetGFHandle, tool.m_matchedTruthJetGF_recoKey);

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

  else if (!m_isTruthJetCol && m_truthLabelConfig == TruthLabelConfiguration::R4TruthLabel) {
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
    
  SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetCollectionName, ctx);
  SG::ReadHandle<xAOD::JetContainer> truthGroomedJets;
  if ( m_getTruthGroomedJetValues ) {
    truthGroomedJets = SG::makeHandle(m_truthGroomedJetCollectionName, ctx);
  }
  const SG::AuxElement::Accessor<int> nbAcc (m_truthLabelName + "_NB");
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
    float ghostFracNominal = 9999;
    const xAOD::Jet* matchTruthJet = nullptr;
    float dRminGroomed = 9999;
    const xAOD::Jet* matchTruthGroomedJet = nullptr;

    // Ensure that the reco jet has at least one constituent
    // (and thus a well-defined four-vector)
    if(jet->numConstituents() > 0){
      for ( const xAOD::Jet* truthJet : *truthJets ) {
          // Calculate DR and GF values. Both are used regardless of the matching option.
          // DR
          float dR = jet->p4().DeltaR(truthJet->p4());
          /// If parent jet has been retrieved, calculate dR w.r.t. it instead
          if (parent) dR = parent->p4().DeltaR(truthJet->p4());
          // GF
          static const SG::ConstAccessor<float> accGhostTruthPt("GhostTruthPt");
          float ghostTruthPt = accGhostTruthPt(*jet);
          float ghostPtFraction = (ghostTruthPt / (truthJet->pt()));
          if (m_useGhostJetMatch) {
              // GA matching. Upper bound applied for completeness, but realistically not needed.
              if ((ghostPtFraction >= m_recoGhostFrac) && (ghostPtFraction <= (2 - m_recoGhostFrac))) {
                  if (std::abs(1 - ghostPtFraction) < std::abs(1 - ghostFracNominal)) {
                      matchTruthJet = truthJet;
                      ghostFracNominal = ghostPtFraction;
                  }
              }
          }
          else {
              // If m_dRTruthJet < 0, the closest truth jet is used as matched jet. Otherwise, only match if dR < m_dRTruthJet
              if (m_dRTruthJet < 0 || dR < m_dRTruthJet ) {
                  if (dR < dRmin) {
                      dRmin = dR;
                      matchTruthJet = truthJet;
                  }
              }
          }

      }
    }


    int label = LargeRJetTruthLabel::enumToInt( LargeRJetTruthLabel::notruth );
    int truthJetNB = -1;
    float truthJetSplit12 = -9999;
    float truthJetSplit23 = -9999;

    float truthJetMass = -9999;
    float truthJetPt = -9999;
    float truthJetEta = -9999;
    float truthJetPhi = -9999;
    float deltaR = -2;
    float ghostFrac = -2;

    if ( matchTruthJet ) {
        // Can't use the WriteDecorHandle to read --- the decoration may have
        // been added and locked by a previous algorithm.
        // Not saving Truth jet decorations for small R
        if (!(m_truthLabelConfig == TruthLabelConfiguration::R4TruthLabel)) {
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
        // Reco jet decorations, saved for small and large R
        truthJetMass = matchTruthJet->m();
        truthJetPt = matchTruthJet->pt();
        truthJetEta = matchTruthJet->eta();
        truthJetPhi = matchTruthJet->phi();
        ghostFrac = ghostFracNominal;
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
      if ( !matchTruthGroomedJet ) {
        for ( const xAOD::Jet* truthGroomedJet : *truthGroomedJets ) {
	        float dR = parent->p4().DeltaR(truthGroomedJet->p4());
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
    if (!(m_truthLabelConfig == TruthLabelConfiguration::R4TruthLabel)) {
        (*dh.labelRecoHandle)(*jet) = label;
        /// Decorate additional information used for truth labeling
        if (m_truthLabelConfig == TruthLabelConfiguration::R21Precision_2022v1 || m_truthLabelConfig == TruthLabelConfiguration::R10TruthLabel_R22v1 || m_truthLabelConfig == TruthLabelConfiguration::R10WZTruthLabel_R22v1) {
            (*dh.split23Handle)(*jet) = truthJetSplit23;
            (*dh.split12Handle)(*jet) = truthJetSplit12;
        }

        (*dh.nbRecoHandle)(*jet) = truthJetNB;
    }
    
    (*dh.matchedTruthJetMassHandle)(*jet) = truthJetMass;
    (*dh.matchedTruthJetPtHandle)(*jet) = truthJetPt;
    (*dh.matchedTruthJetEtaHandle)(*jet) = truthJetEta;
    (*dh.matchedTruthJetPhiHandle)(*jet) = truthJetPhi;
    (*dh.matchedTruthJetDRHandle)(*jet) = deltaR;
    (*dh.matchedTruthJetGFHandle)(*jet) = ghostFrac;

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
  SG::ReadHandle<xAOD::JetContainer> truthJets(m_truthJetCollectionName, ctx);

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

  int label = LargeRJetTruthLabel::enumToInt(LargeRJetTruthLabel::notruth);
  /// Apply label to truth jet
  for ( const xAOD::Jet *jet : truthJets ) {
    ATH_MSG_DEBUG("Getting truth label using ghost-association");
    label = getTruthJetLabelGA(dh, *jet, ctx);
    (*dh.labelHandle)(*jet) = label;
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

int JetTruthLabelingTool::getNGhostParticles( const xAOD::Jet &jet, std::string collection ) const {

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
      ATH_MSG_ERROR( "HadronGhostExtendedTruthLabelID not available for " + m_truthJetCollectionName.key() );
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
