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

StatusCode JetTruthLabelingTool::initialize(){

  ATH_MSG_INFO("Initializing " << name());

  /// Hard-code some values for R10TruthLabel_R21Consolidated                                                                                                                                             
  /// Functionality to customize labeling will be added later
  if(m_truthLabelName == "R10TruthLabel_R21Consolidated") {
    m_truthJetCollectionName="AntiKt10TruthTrimmedPtFrac5SmallR20Jets";
    m_matchUngroomedParent = false;
    m_dRTruthJet = 0.75;
    m_useDRMatch = true;
    m_dRTruthPart = 0.75;
    m_useWZMassHigh = true;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mHighW = 100.0;
    m_mLowZ = 60.0;
    m_mHighZ = 110.0;
    m_getTruthGroomedJetValues = false;
  }
  /// Hard-code some values for R10TruthLabel_R21Precision                                                                                                                                                
  else if(m_truthLabelName == "R10TruthLabel_R21Precision") {
    m_truthJetCollectionName="AntiKt10TruthJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_useDRMatch = false;
    m_useWZMassHigh = true;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mHighW = 100.0;
    m_mLowZ = 60.0;
    m_mHighZ = 110.0;
    m_getTruthGroomedJetValues = false;
  }
  /// Hard-code some values for R10TruthLabel_R21Precision_2022v1 and R10TruthLabel_R22v1                                                                                                     
  else if( m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" or m_truthLabelName == "R10TruthLabel_R22v1" ) {
    m_truthJetCollectionName="AntiKt10TruthJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_useDRMatch = false;
    m_useWZMassHigh = false;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mLowZ = 50.0;
    if ( m_truthLabelName == "R10TruthLabel_R22v1" ) {
      m_getTruthGroomedJetValues = true;
      m_truthGroomedJetCollectionName = "AntiKt10TruthSoftDropBeta100Zcut10Jets";
    } else {
      m_getTruthGroomedJetValues = false;
    }
  }
  /// Hard-code some values for R10WZTruthLabel_R22v1
  else if( m_truthLabelName == "R10WZTruthLabel_R22v1" ) {
    m_truthJetCollectionName="AntiKt10TruthDressedWZJets";
    m_matchUngroomedParent = true;
    m_dRTruthJet = 0.75;
    m_useDRMatch = false;
    m_useWZMassHigh = false;
    m_mLowTop = 140.0;
    m_mLowW = 50.0;
    m_mLowZ = 50.0;
    m_getTruthGroomedJetValues = true;
    m_truthGroomedJetCollectionName = "AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets";
  } 

  print();

  /// Check if TruthLabelName is supported. If not, give an error and return FAILURE
  
  bool isSupportedLabel = false;
  isSupportedLabel = isSupportedLabel || (m_truthLabelName=="R10TruthLabel_R21Consolidated");
  isSupportedLabel = isSupportedLabel || (m_truthLabelName=="R10TruthLabel_R21Precision");
  isSupportedLabel = isSupportedLabel || (m_truthLabelName=="R10TruthLabel_R21Precision_2022v1");
  isSupportedLabel = isSupportedLabel || (m_truthLabelName=="R10TruthLabel_R22v1");
  isSupportedLabel = isSupportedLabel || (m_truthLabelName=="R10WZTruthLabel_R22v1");

  if(!isSupportedLabel) {
    ATH_MSG_ERROR("TruthLabelName " << m_truthLabelName << " is not supported. Exiting...");
    return StatusCode::FAILURE;
  }

  m_label_truthKey   = m_truthJetCollectionName.key() + "." + m_truthLabelName;
  m_dR_W_truthKey    = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_dR_W";
  m_dR_Z_truthKey    = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_dR_Z";
  m_dR_H_truthKey    = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_dR_H";
  m_dR_Top_truthKey  = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_dR_Top";
  m_NB_truthKey      = m_truthJetCollectionName.key() + "." + m_truthLabelName + "_NB";
  m_split12_truthKey = m_truthJetCollectionName.key() + ".Split12";
  m_split23_truthKey = m_truthJetCollectionName.key() + ".Split23";

  if(!m_isTruthJetCol){
    m_label_recoKey  = m_jetContainerName + "." + m_truthLabelName;
    m_dR_W_recoKey   = m_jetContainerName + "." + m_truthLabelName + "_dR_W";
    m_dR_Z_recoKey   = m_jetContainerName + "." + m_truthLabelName + "_dR_Z";
    m_dR_H_recoKey   = m_jetContainerName + "." + m_truthLabelName + "_dR_H";
    m_dR_Top_recoKey = m_jetContainerName + "." + m_truthLabelName + "_dR_Top";
    m_NB_recoKey     = m_jetContainerName + "." + m_truthLabelName + "_NB";
    m_truthSplit12_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit12";
    m_truthSplit23_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetSplit23";
    m_truthJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetMass";
    m_truthJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthJetPt";
    m_truthGroomedJetMass_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthGroomedJetMass";
    m_truthGroomedJetPt_recoKey = m_jetContainerName + "." + m_truthLabelName + "_TruthGroomedJetPt";
  }

  ATH_CHECK(m_evtInfoKey.initialize());
  ATH_CHECK(m_truthParticleContainerName.initialize(!m_useTRUTH3));
  ATH_CHECK(m_truthBosonContainerName.initialize(m_useTRUTH3));
  ATH_CHECK(m_truthTopQuarkContainerName.initialize(m_useTRUTH3));
  ATH_CHECK(m_truthJetCollectionName.initialize());
  ATH_CHECK(m_truthGroomedJetCollectionName.initialize(m_getTruthGroomedJetValues));

  ATH_CHECK(m_label_truthKey.initialize());
  ATH_CHECK(m_dR_W_truthKey.initialize(m_useDRMatch));
  ATH_CHECK(m_dR_Z_truthKey.initialize(m_useDRMatch));
  ATH_CHECK(m_dR_H_truthKey.initialize(m_useDRMatch));
  ATH_CHECK(m_dR_Top_truthKey.initialize(m_useDRMatch));
  ATH_CHECK(m_NB_truthKey.initialize());
  
  ATH_CHECK(m_split12_truthKey.initialize(m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1"));
  ATH_CHECK(m_split23_truthKey.initialize(m_truthLabelName == "R10TruthLabel_R21Precision" || m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1"));

  ATH_CHECK(m_label_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_dR_W_recoKey.initialize(!m_isTruthJetCol && m_useDRMatch));
  ATH_CHECK(m_dR_Z_recoKey.initialize(!m_isTruthJetCol && m_useDRMatch));
  ATH_CHECK(m_dR_H_recoKey.initialize(!m_isTruthJetCol && m_useDRMatch));
  ATH_CHECK(m_dR_Top_recoKey.initialize(!m_isTruthJetCol && m_useDRMatch));
  ATH_CHECK(m_NB_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_truthSplit12_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1") ));
  ATH_CHECK(m_truthSplit23_recoKey.initialize(!m_isTruthJetCol && (m_truthLabelName == "R10TruthLabel_R21Precision" || m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1")));
  ATH_CHECK(m_truthJetMass_recoKey.initialize(!m_isTruthJetCol));
  ATH_CHECK(m_truthJetPt_recoKey.initialize(!m_isTruthJetCol));

  ATH_CHECK(m_truthGroomedJetMass_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));
  ATH_CHECK(m_truthGroomedJetPt_recoKey.initialize((!m_isTruthJetCol) && (m_getTruthGroomedJetValues)));

  return StatusCode::SUCCESS;
}

void JetTruthLabelingTool::print() const {
  ATH_MSG_INFO("Parameters for " << name());

  ATH_MSG_INFO("xAOD information:");
  ATH_MSG_INFO("TruthLabelName:               " << m_truthLabelName);
  ATH_MSG_INFO("UseTRUTH3:                    " << ( m_useTRUTH3 ? "True" : "False"));
  
  if(m_useTRUTH3) {
    ATH_MSG_INFO("TruthBosonContainerName:      " << m_truthBosonContainerName);
    ATH_MSG_INFO("TruthTopQuarkContainerName:   " << m_truthTopQuarkContainerName);
  }
  else {
    ATH_MSG_INFO("TruthParticleContainerName:   " << m_truthParticleContainerName);
  }
  
  ATH_MSG_INFO("TruthJetCollectionName:         " << m_truthJetCollectionName.key());
  ATH_MSG_INFO("dRTruthJet:    " << std::to_string(m_dRTruthJet));

  if ( m_useDRMatch ) {
    ATH_MSG_INFO("dRTruthPart:   " << std::to_string(m_dRTruthPart));
  }

  ATH_MSG_INFO("mLowTop:       " << std::to_string(m_mLowTop));
  ATH_MSG_INFO("mLowW:         " << std::to_string(m_mLowW));
  if(m_useWZMassHigh)
    ATH_MSG_INFO("mHighW:        " << std::to_string(m_mHighW));
  ATH_MSG_INFO("mLowZ:         " << std::to_string(m_mLowZ));
  if(m_useWZMassHigh)
    ATH_MSG_INFO("mHighZ:        " << std::to_string(m_mHighZ));

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
  maybeInit (dRWHandle,       tool.m_dR_W_truthKey);
  maybeInit (dRZHandle,       tool.m_dR_Z_truthKey);
  maybeInit (dRHHandle,       tool.m_dR_H_truthKey);
  maybeInit (dRTopHandle,     tool.m_dR_Top_truthKey);
  maybeInit (dRWRecoHandle,   tool.m_dR_W_recoKey);
  maybeInit (dRZRecoHandle,   tool.m_dR_Z_recoKey);
  maybeInit (dRHRecoHandle,   tool.m_dR_H_recoKey);
  maybeInit (dRTopRecoHandle, tool.m_dR_Top_recoKey);
  maybeInit (split23Handle,   tool.m_truthSplit23_recoKey);
  maybeInit (split12Handle,   tool.m_truthSplit12_recoKey);
  maybeInit (truthMassHandle, tool.m_truthJetMass_recoKey);
  maybeInit (truthPtHandle,   tool.m_truthJetPt_recoKey);
  maybeInit (truthGroomedMassHandle, tool.m_truthGroomedJetMass_recoKey);
  maybeInit (truthGroomedPtHandle,   tool.m_truthGroomedJetPt_recoKey);
}

int JetTruthLabelingTool::getTruthJetLabelDR( DecorHandles& dh,
                                              const xAOD::Jet &jet,
                                              const std::vector<std::pair<TLorentzVector,int> >& tlv_truthParts,
                                              const EventContext& ctx) const {

  /// Booleans to check associated heavy particles
  bool matchW = false;
  bool matchZ = false;
  bool matchH = false;
  bool matchTop = false;

  /// Distances to truth particles
  float dR_W = 9999;
  float dR_Z = 9999;
  float dR_H = 9999;
  float dR_Top = 9999;

  for (const auto& tlv_truth : tlv_truthParts) {
    float dR = tlv_truth.first.DeltaR(jet.p4());
    if( dR < m_dRTruthPart ) {

      if ( MC::isZ(tlv_truth.second) && !matchZ ) {
        dR_Z = dR;
        matchZ = true;
      }

      if ( MC::isW(tlv_truth.second) && !matchW ) {
        dR_W = dR;
        matchW = true;
      }

      if ( MC::isHiggs(tlv_truth.second) && !matchH ) {
        dR_H = dR;
        matchH = true;
      }

      if ( MC::isTop(tlv_truth.second) && !matchTop ) {
        dR_Top = dR;
        matchTop = true;
      }

    }
  }

  /// Add matching criteria decorations
  (*dh.dRWHandle)(jet) = dR_W;
  (*dh.dRZHandle)(jet) = dR_Z;
  (*dh.dRHHandle)(jet) = dR_H;
  (*dh.dRTopHandle)(jet) = dR_Top;

  return getLabel( dh, jet, matchH, matchW, matchZ, matchTop, ctx );

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
    const xAOD::Jet* matchTruthJet = nullptr;
    float dRminGroomed = 9999;
    const xAOD::Jet* matchTruthGroomedJet = nullptr;

    // Ensure that the reco jet has at least one constituent
    // (and thus a well-defined four-vector)
    if(jet->numConstituents() > 0){
      for ( const xAOD::Jet* truthJet : *truthJets ) {
	float dR = jet->p4().DeltaR( truthJet->p4() );
	/// If parent jet has been retrieved, calculate dR w.r.t. it instead
	if ( parent ) dR = parent->p4().DeltaR( truthJet->p4() );
	/// If m_dRTruthJet < 0, the closest truth jet is used as matched jet. Otherwise, only match if dR < m_dRTruthJet
	if ( m_dRTruthJet < 0 || dR < m_dRTruthJet ) {
	  if ( dR < dRmin ) {
	    dRmin = dR;
	    matchTruthJet = truthJet;
	  }
	}
      }
    }

    int label = LargeRJetTruthLabel::enumToInt( LargeRJetTruthLabel::notruth );
    float dR_truthJet_W = 9999;
    float dR_truthJet_Z = 9999;
    float dR_truthJet_Top = 9999;
    float dR_truthJet_H = 9999;
    int truthJetNB = -1;
    float truthJetSplit12 = -9999;
    float truthJetSplit23 = -9999;
    float truthJetMass = -9999;
    float truthJetPt = -9999;

    if ( matchTruthJet ) {
      // Can't use the WriteDecorHandle to read --- the decoration may have
      // been added and locked by a previous algorithm.
      SG::ConstAccessor<int> labelAcc (dh.labelHandle->auxid());
      label = labelAcc(*matchTruthJet);
      if ( m_useDRMatch ) {
        if(dh.dRWHandle->isAvailable()) dR_truthJet_W = (*dh.dRWHandle)(*matchTruthJet);
        if(dh.dRZHandle->isAvailable()) dR_truthJet_Z = (*dh.dRZHandle)(*matchTruthJet);
        if(dh.dRHHandle->isAvailable()) dR_truthJet_H = (*dh.dRHHandle)(*matchTruthJet);
        if(dh.dRTopHandle->isAvailable()) dR_truthJet_Top = (*dh.dRTopHandle)(*matchTruthJet);
      }
      if ( m_truthLabelName == "R10TruthLabel_R21Precision" ) {
        SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
        if(split23Handle.isAvailable()) truthJetSplit23 = split23Handle(*matchTruthJet);
      }
      if ( m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1" ) {
        SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
        if(split23Handle.isAvailable()) truthJetSplit23 = split23Handle(*matchTruthJet);
        SG::ReadDecorHandle<xAOD::JetContainer, float> split12Handle(m_split12_truthKey, ctx);
        if(split12Handle.isAvailable()) truthJetSplit12 = split12Handle(*matchTruthJet);
      }
      if(nbAcc.isAvailable(*matchTruthJet)) truthJetNB = nbAcc (*matchTruthJet);
      truthJetMass = matchTruthJet->m();
      truthJetPt = matchTruthJet->pt();
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
	        float dR = jet->p4().DeltaR( truthGroomedJet->p4() );
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
    (*dh.labelRecoHandle)(*jet) = label;

    /// Decorate additional information used for truth labeling
    if ( m_useDRMatch ) {
      (*dh.dRWRecoHandle)(*jet) = dR_truthJet_W;
      (*dh.dRZRecoHandle)(*jet) = dR_truthJet_Z;
      (*dh.dRHRecoHandle)(*jet) = dR_truthJet_H;
      (*dh.dRTopRecoHandle)(*jet) = dR_truthJet_Top;
    }
    if ( m_truthLabelName == "R10TruthLabel_R21Precision" ) {
      (*dh.split23Handle)(*jet) = truthJetSplit23;
    }

    if ( m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1" ) {
      (*dh.split23Handle)(*jet) = truthJetSplit23;
      (*dh.split12Handle)(*jet) = truthJetSplit12;
    }

    (*dh.nbRecoHandle)(*jet) = truthJetNB;
    (*dh.truthMassHandle)(*jet) = truthJetMass;
    (*dh.truthPtHandle)(*jet) = truthJetPt;

    if ( m_getTruthGroomedJetValues ) {
      (*dh.truthGroomedMassHandle)(*jet) = truthGroomedJetMass;
      (*dh.truthGroomedPtHandle)(*jet) = truthGroomedJetPt;
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

  /// Get MC channel number
  int channelNumber = -999;

  /// Get the EventInfo to identify Sherpa samples
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfoKey, ctx);
  if(!eventInfo.isValid()){
    ATH_MSG_ERROR("Failed to retrieve event information.");
    return StatusCode::FAILURE;
  }

  /// Vectors of W/Z/H/Top TLorentzVectors
  std::vector<std::pair<TLorentzVector,int> > tlv_truthParts;

  /// Get truth particles directly if using dR matching
  if ( m_useDRMatch ) {
  
    channelNumber = eventInfo->mcChannelNumber();

    if ( channelNumber < 0 ) {
      ATH_MSG_ERROR("Channel number was not set correctly");
      return StatusCode::FAILURE;
    }

    /// Check if it is a Sherpa sample
    bool isSherpa = getIsSherpa(channelNumber);

    if ( m_useTRUTH3 && isSherpa ) {
      ATH_MSG_ERROR("Cannot apply truth labels to Sherpa 2.2.1 samples using TRUTH3 containers");
      return StatusCode::FAILURE;
    }

    /// TRUTH3
    if( m_useTRUTH3 ) {
      /// Check for boson container
      SG::ReadHandle<xAOD::TruthParticleContainer> truthPartsBoson(m_truthBosonContainerName, ctx);
      if(!truthPartsBoson.isValid()){
        ATH_MSG_ERROR("Unable to find " << m_truthBosonContainerName.key() << ". Please check the content of your input file.");
        return StatusCode::FAILURE;
      }
      /// Check for top quark container
      SG::ReadHandle<xAOD::TruthParticleContainer> truthPartsTop(m_truthTopQuarkContainerName, ctx);
      if(!truthPartsTop.isValid()){
        ATH_MSG_ERROR("Unable to find " << m_truthTopQuarkContainerName.key() << ". Please check the content of your input file.");
        return StatusCode::FAILURE;
      }
      /// Get truth particle TLVs
      getTLVs(tlv_truthParts, truthPartsBoson.cptr(), truthPartsTop.cptr(), isSherpa);
    }

    /// TRUTH1
    else {
      SG::ReadHandle<xAOD::TruthParticleContainer> truthParts(m_truthParticleContainerName, ctx);
      if(!truthParts.isValid()){
        ATH_MSG_ERROR("Unable to find " << m_truthParticleContainerName << ". Please check the content of your input file.");
        return StatusCode::FAILURE;
      }
      /// Get truth particle TLVs
      getTLVs(tlv_truthParts, truthParts.cptr(), truthParts.cptr(), isSherpa);
    }
  }

  int label = LargeRJetTruthLabel::enumToInt(LargeRJetTruthLabel::notruth);
  /// Apply label to truth jet
  for ( const xAOD::Jet *jet : truthJets ) {

    if ( m_useDRMatch ) {
      ATH_MSG_DEBUG("Getting truth label using dR matching");
      label = getTruthJetLabelDR(dh, *jet, tlv_truthParts, ctx);
    }
    
    else {
      ATH_MSG_DEBUG("Getting truth label using ghost-association");
      label = getTruthJetLabelGA(dh, *jet, ctx);
    }

    (*dh.labelHandle)(*jet) = label;
  }

  return StatusCode::SUCCESS;
}

void JetTruthLabelingTool::getTLVs( std::vector<std::pair<TLorentzVector,int> > &tlvs, const xAOD::TruthParticleContainer *truthBosons, const xAOD::TruthParticleContainer *truthTop, bool isSherpa ) const {

  tlvs.clear();

  /// Sherpa W/Z+jets samples need special treatment
  if(isSherpa) {
    int countStatus3 = 0;

    /// Decay products
    TLorentzVector p1(0,0,0,0);
    TLorentzVector p2(0,0,0,0);

    /// W candidate
    TLorentzVector WZCand(0,0,0,0);

    /// Flag for W or Z candidates
    bool isWPCand = false;
    bool isWMCand = false;
    bool isZCand = false;

    /// Flag to check if qq' pair is in the mass window
    bool inMassWindow = false;

    for ( unsigned int ipart = 0; ipart < truthBosons->size(); ipart++ ){

      const xAOD::TruthParticle* part1 = truthBosons->at(ipart);

      /// Skip decayed/stable
      if ( MC::isPhysical(part1) ) continue;

      /// Skip anything that isn't a light quark
      if ( std::abs(part1->pdgId()) > MC::BQUARK ) continue;

      countStatus3++;
      /// We want to look at first 2 partons except beam particles. Sometimes beam particles are dropped from DxAODs...
      if ( countStatus3 > 3 ) continue;
      /// Keep particles as first daughter
      p1 = part1->p4();

      /// Find the next particle in the list with status == 3
      for ( unsigned int jpart = ipart+1; jpart < truthBosons->size(); jpart++ ) {

        const xAOD::TruthParticle* part2 = truthBosons->at(jpart);

        /// Skip decayed/stable
        if (  MC::isPhysical(part2) ) continue;

        /// Skip anything that isn't a light quark
        if ( std::abs(part2->pdgId()) > MC::BQUARK ) continue;

        p2 = part2->p4();

        /// Daughters of Z decay should be same-flavor but opposite charge
        if ( part1->pdgId() + part2->pdgId() == 0 ) {
          isZCand = true;
        }
        /// W+ daughters should have a positive u or c
        else if ( part1->pdgId() == MC::UQUARK || part1->pdgId() == MC::CQUARK || part2->pdgId() == MC::UQUARK || part2->pdgId() == MC::CQUARK ) {
          isWPCand = true;
        }
        /// W+ daughters should have a positive u or c
        else {
          isWMCand = true;
        }

        /// If p1 is a daughter of W/Z decay, the next one is the other daughter
        break;

      }

      WZCand = p1 + p2;

      /// ~98% efficiency to W/Z signals. (95% if it is changed to [65, 105]GeV and 90% if [70,100]GeV)
      if ( 60000 < WZCand.M() && WZCand.M() < 140000. ) {
        inMassWindow = true;
        break;
      }

    }

    if ( inMassWindow && (isWPCand || isWMCand || isZCand) ) {
      std::pair<TLorentzVector,int> WZ;
      if ( isZCand ) {
        WZ = std::make_pair(WZCand,23);
      }
      if ( isWPCand ) {
        WZ = std::make_pair(WZCand,24);
      }
      if ( isWMCand ) {
        WZ = std::make_pair(WZCand,-24);
      }
      tlvs.push_back(WZ);
    }

  }

  /// Store W/Z/H bosons
  for ( const xAOD::TruthParticle* part : *truthBosons ) {
    if ( !(selectTruthParticle(part,MC::Z0BOSON) || selectTruthParticle(part,MC::WPLUSBOSON) || selectTruthParticle(part,MC::HIGGSBOSON)) ) continue;
    /// Save 4-vector and pdgId
    tlvs.push_back(std::make_pair(part->p4(),part->pdgId()));
  }

  /// Store top quarks
  for ( const xAOD::TruthParticle* part : *truthTop ) {
    if ( !selectTruthParticle(part,MC::TQUARK) ) continue;
    /// Save 4-vector and pdgId
    tlvs.push_back(std::make_pair(part->p4(),part->pdgId()));
  }

}

bool JetTruthLabelingTool::selectTruthParticle( const xAOD::TruthParticle *tp, int pdgId ) const {
  if ( std::abs(tp->pdgId()) != pdgId ) return false;
  for ( unsigned int iChild = 0; iChild < tp->nChildren(); iChild++ ) {
    const xAOD::TruthParticle *child = tp->child(iChild);
    if ( !child ) continue;
    if ( child->pdgId() == tp->pdgId() ) return false;
  }
  return true;
}

float JetTruthLabelingTool::getWZSplit12Cut( float pt ) const {

  /// The functional form and parameters come from optimization studies:
  /// https://cds.cern.ch/record/2777009/files/ATL-PHYS-PUB-2021-029.pdf

  float split12 = -999.0;

  if ( m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || m_truthLabelName == "R10TruthLabel_R22v1" || m_truthLabelName == "R10WZTruthLabel_R22v1") {
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

  if ( m_truthLabelName == "R10TruthLabel_R21Precision" || 
       m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" || 
       m_truthLabelName == "R10TruthLabel_R22v1" || 
       m_truthLabelName == "R10WZTruthLabel_R22v1") {

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

  /// Use R21Consolidated definition
  if ( m_truthLabelName == "R10TruthLabel_R21Consolidated" ) {
    is_bb = ( nMatchB > 1 );
    isTop = ( matchTop && nMatchB > 0 && jet.m() / 1000. > m_mLowTop );
    isW = matchW && nMatchB == 0 && jet.m() / 1000. > m_mLowW && jet.m() / 1000. < m_mHighW;
    isZ = matchZ && jet.m() / 1000. > m_mLowZ && jet.m() / 1000. < m_mHighZ;
  }

  /// Use R21Precision definition
  if ( m_truthLabelName == "R10TruthLabel_R21Precision" ) {
    SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
    is_bb = ( nMatchB > 1 );
    isTop = ( matchTop && matchW && nMatchB > 0 && jet.m() / 1000. > m_mLowTop && split23Handle(jet) / 1000. > getTopSplit23Cut( jet.pt() / 1000. ) );
    isW = matchW && nMatchB == 0 && jet.m() / 1000. > m_mLowW && jet.m() / 1000. < m_mHighW;
    isZ = matchZ && jet.m() / 1000. > m_mLowZ && jet.m() / 1000. < m_mHighZ;
  }

  // Use R21Precision_2022v1 definition
  if ( m_truthLabelName == "R10TruthLabel_R21Precision_2022v1" ) {
    SG::ReadDecorHandle<xAOD::JetContainer, float> split23Handle(m_split23_truthKey, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, float> split12Handle(m_split12_truthKey, ctx);
    is_bb = ( nMatchB > 1 );
    isTop = ( matchTop && matchW && nMatchB > 0 && jet.m() / 1000. > m_mLowTop && split23Handle(jet) / 1000. > getTopSplit23Cut( jet.pt() / 1000. ) );
    isW = matchW && nMatchB == 0 && jet.m() / 1000. > m_mLowW && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
    isZ = matchZ && jet.m() / 1000. > m_mLowZ && split12Handle(jet) / 1000. > getWZSplit12Cut( jet.pt() / 1000. );
  }

  // Use R10TruthLabel_R22v1 definition
  if ( m_truthLabelName == "R10TruthLabel_R22v1"  || m_truthLabelName == "R10WZTruthLabel_R22v1") {
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
   * 4) Contained Zbb/cc/qq
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
