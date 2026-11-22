/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JSSTaggerSelectionTool.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include <cmath>

namespace CP {

  StatusCode JSSTaggerSelectionTool::initialize() {

    if (m_passTaggerKey.key().empty() || m_passValidKinRangeKey.key().empty() || m_passScoreKey.key().empty()) {
      ATH_MSG_ERROR("TaggedName, ValidKinRangeName and PassScoreName must all be set");
      return StatusCode::FAILURE;
    }

    ATH_CHECK(m_cutTool.retrieve());

    m_etaAcc = SG::ConstAccessor<float>(m_jetEtaName);

    m_info.addCut("PassKinRange", "Whether the jet passes the kinematic selection");
    m_info.addCut("PassScore", "Whether the jet passes the JSS tagger score cut");

    ATH_CHECK(m_jetContainer.initialize());
    ATH_CHECK(m_scoreNameKey.initialize());
    ATH_CHECK(m_passTaggerKey.initialize());
    ATH_CHECK(m_passValidKinRangeKey.initialize());
    ATH_CHECK(m_passScoreKey.initialize());

    ATH_MSG_INFO("  " << m_passTaggerKey.key() << " : pass tagger (kinematics + score)");
    ATH_MSG_INFO("  " << m_passValidKinRangeKey.key() << " : pass kinematic range");
    ATH_MSG_INFO("  " << m_passScoreKey.key() << " : pass score cut");

    return StatusCode::SUCCESS;
  }

  const asg::AcceptInfo& JSSTaggerSelectionTool::getAcceptInfo() const {
    return m_info;
  }

  asg::AcceptData JSSTaggerSelectionTool::accept(const xAOD::IParticle* jet) const {
    asg::AcceptData data(&m_info);
    SG::ReadDecorHandle<xAOD::JetContainer, float> scoreHandle(m_scoreNameKey);
    data.setCutResult("PassKinRange", isInRange(*jet));
    data.setCutResult("PassScore", passScore(*jet, scoreHandle(*jet)));
    return data;
  }

  bool JSSTaggerSelectionTool::isInRange(const xAOD::IParticle& jet) const {
    const float eta = m_etaAcc(jet);
    return jet.pt() >= m_jetPtMin && jet.pt() <= m_jetPtMax && std::abs(eta) <= m_jetEtaMax;
  }

  bool JSSTaggerSelectionTool::passScore(const xAOD::IParticle& jet, float score) const {
    JetHelper::JetContext jc;
    const float cut = m_cutTool->getValue(static_cast<const xAOD::Jet&>(jet), jc);
    ATH_MSG_DEBUG("jet pt: " << jet.pt() << ", eta: " << jet.eta() << ", score: " << score << ", cut: " << cut);
    return score > cut;
  }

  StatusCode JSSTaggerSelectionTool::decorate(const xAOD::JetContainer& jets) const {

    SG::ReadDecorHandle<xAOD::JetContainer, float> scoreHandle(m_scoreNameKey);
    SG::WriteDecorHandle<xAOD::JetContainer, char> passTaggerHandle(m_passTaggerKey);
    SG::WriteDecorHandle<xAOD::JetContainer, char> passKinHandle(m_passValidKinRangeKey);
    SG::WriteDecorHandle<xAOD::JetContainer, char> passScoreHandle(m_passScoreKey);

    for (const xAOD::Jet* jet : jets) {
      const bool passKin = isInRange(*jet);
      const bool passCut = passScore(*jet, scoreHandle(*jet));

      passKinHandle(*jet) = passKin;
      passScoreHandle(*jet) = passCut;
      passTaggerHandle(*jet) = passKin && passCut;
    }

    return StatusCode::SUCCESS;
  }

} // namespace CP
