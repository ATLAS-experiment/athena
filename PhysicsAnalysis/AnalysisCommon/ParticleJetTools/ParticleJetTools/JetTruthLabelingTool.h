/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETTOOLS_JETTRUTHLABELINGTOOL_H
#define PARTICLEJETTOOLS_JETTRUTHLABELINGTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "AthLinks/ElementLink.h"
#include "JetInterface/IJetDecorator.h"
#include "ParticleJetTools/LargeRJetLabelEnum.h"
#include <optional>

class JetTruthLabelingTool :
  public asg::AsgTool,
  virtual public IJetDecorator
{
  ASG_TOOL_CLASS(JetTruthLabelingTool, IJetDecorator)

public:

  /// default constructor - to be used in all derived classes
  JetTruthLabelingTool(const std::string& name = "JetTruthLabelingTool");
  virtual StatusCode initialize() override;

  /// decorate truth label to a jet collection
  StatusCode decorate(const xAOD::JetContainer& jets) const override;

  /// Print configured parameters
  void print() const override;
  
  /// returns the name of large-R jet truth label
  const std::string& getLargeRJetTruthLabelName() const {
    return m_truthLabelName;
  };

protected:
  
  Gaudi::Property<std::string> m_jetContainerName{this, "RecoJetContainer", "", "Input reco jet container name"};
  Gaudi::Property<std::string> m_truthLabelName{this, "TruthLabelName", "R10TruthLabel_R22v1", "Truth label name"};

  Gaudi::Property<bool> m_isTruthJetCol{this, "IsTruthJetCollection", false, "Flag indicating whether input collection is a truth jet container"};
  Gaudi::Property<bool> m_forceDeltaRMatch{this, "ForceDeltaRMatch", false, "Whether to force dR matching e.g. if GhostTruth is not available"};

  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EventInfoKey", "EventInfo", "Name of EventInfo object"};

  /// parameters for truth labeling
  SG::ReadHandleKey<xAOD::JetContainer> m_truthJetCollectionKey{this, "TruthJetContainer", "", "Do not configure manually!"};
  SG::ReadHandleKey<xAOD::JetContainer> m_truthGroomedJetCollectionKey{this, "TruthGroomedJetContainer", "", "Do not configure manually!"};
  bool m_useGhostJetMatch{}; /// Use ghost association to match reco to truth jets, dR otherwise
  bool m_matchUngroomedParent{}; /// Use the ungroomed reco jet parent to match to truth jet
  bool m_getTruthGroomedJetValues{}; /// When truth jet matching to ungroomed truth, allow saving properties of groomed truth jets
  double m_dRTruthJet{}; /// dR to match truth jet to reco jet
  double m_recoGhostFrac{}; /// Ghost pT fraction to match truth jet to reco jet
  double m_mLowTop{}; /// Lower mass cut for top label
  double m_mLowW{}; /// Lower mass cut for W label
  double m_mLowZ{}; /// Lower mass cut for Z label
  bool m_doLargeRLabels{true}; /// Track internally if the W,Z,Top labels should be done (for large-R)

  struct DecorHandles {
    DecorHandles (const JetTruthLabelingTool& tool, const EventContext& ctx);

    using IntHandle_t = SG::WriteDecorHandle<xAOD::JetContainer, int>;
    using IntHandleOp_t = std::optional<IntHandle_t>;
    IntHandleOp_t labelHandle;
    IntHandleOp_t nbHandle;
    IntHandleOp_t labelRecoHandle;
    IntHandleOp_t nbRecoHandle;
    using FloatHandle_t = SG::WriteDecorHandle<xAOD::JetContainer, float>;
    using FloatHandleOp_t = std::optional<FloatHandle_t>;
    FloatHandleOp_t split23Handle;
    FloatHandleOp_t split12Handle;
    using ELHandle_t = SG::WriteDecorHandle<xAOD::JetContainer, ElementLink<xAOD::JetContainer> >;
    using ELHandleOp_t = std::optional<ELHandle_t>;
    ELHandleOp_t matchedTruthJetHandle;
    FloatHandleOp_t matchedTruthJetMassHandle;
    FloatHandleOp_t matchedTruthJetPtHandle;
    FloatHandleOp_t matchedTruthJetEtaHandle;
    FloatHandleOp_t matchedTruthJetPhiHandle;
    FloatHandleOp_t matchedTruthJetDRHandle;
    FloatHandleOp_t matchedTruthJetGFHandle;
    FloatHandleOp_t matchedTruthGroomedMassHandle;
    FloatHandleOp_t matchedTruthGroomedPtHandle;
  };
  friend struct DecorHandles;

  /// Label truth jet collection
  StatusCode labelTruthJets( DecorHandles& dh,
                             const EventContext& ctx ) const;
  StatusCode labelTruthJets( DecorHandles& dh,
                             const xAOD::JetContainer &jets,
                             const EventContext& ctx ) const;

  /// Apply labels to all jets in a container
  StatusCode labelRecoJets(DecorHandles& dh,
                           const xAOD::JetContainer &jets,
                           const EventContext& ctx) const;

  /// Get truth label using ghost-associated particles
  int getTruthJetLabelGA( DecorHandles& dh,
                          const xAOD::Jet &jet,
                          const EventContext& ctx ) const;

  /// Get label based on matching and containment criteria
  int getLabel( DecorHandles& dh,
                const xAOD::Jet &jet, bool matchH, bool matchW, bool matchZ, bool matchTop,
                const EventContext& ctx ) const;

  /// Get W/Z label Split12 cut
  float getWZSplit12Cut( float pt ) const;

  /// Get top label Split23 cut
  float getTopSplit23Cut( float pt ) const;

  /// Get number of ghost associated particles
  int getNGhostParticles( const xAOD::Jet &jet, const std::string & collection ) const;

  enum class TruthLabelConfiguration {
      R21Precision_2022v1,
      R10TruthLabel_R22v1,
      R10WZTruthLabel_R22v1,
      R4TruthLabel,
      R4TruthDressedWZLabel,
      R4InTimeTruthLabel,
      Unknown
  };

  TruthLabelConfiguration parseLabel(const std::string& label);
  TruthLabelConfiguration m_truthLabelConfig = TruthLabelConfiguration::Unknown;

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_label_truthKey{this, "label_TruthKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_NB_truthKey{this, "NB_TruthKey", "", "Do not configure manually!"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_split12_truthKey{this, "Split12_TruthKey", "", "Do not configure manually!"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_split23_truthKey{this, "Split23_TruthKey", "", "Do not configure manually!"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_label_recoKey{this, "label_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_NB_recoKey{this, "NB_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_truthSplit12_recoKey{this, "TruthSplit12_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_truthSplit23_recoKey{this, "TruthSplit23_RecoKey", "", "Do not configure manually!"};

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJet_recoKey{this, "MatchedTruthJet_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetMass_recoKey{this, "MatchedTruthJetMass_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetPt_recoKey{this, "MatchedTruthJetPt_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetEta_recoKey{ this, "MatchedTruthJetEta_RecoKey", "", "Do not configure manually!" };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetPhi_recoKey{ this, "MatchedTruthJetPhi_RecoKey", "", "Do not configure manually!" };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetDR_recoKey{ this, "MatchedTruthJetDR_RecoKey", "", "Do not configure manually!" };
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthJetGF_recoKey{ this, "MatchedTruthJetGF_RecoKey", "", "Do not configure manually!" };

  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthGroomedJetMass_recoKey{this, "MatchedTruthGroomedJetMass_RecoKey", "", "Do not configure manually!"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_matchedTruthGroomedJetPt_recoKey{this, "MatchedTruthGroomedJetPt_RecoKey", "", "Do not configure manually!"};
};

#endif
