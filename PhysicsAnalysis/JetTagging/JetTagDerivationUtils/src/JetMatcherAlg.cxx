/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMatcherAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include <vector>
#include <memory>

namespace ftag {

  JetMatcherAlg::JetMatcherAlg(const std::string& name,
                               ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator)
  {
    declareProperty("floatsToCopy", m_floats.toCopy);
    declareProperty("doublesToCopy", m_doubles.toCopy);
    declareProperty("intsToCopy", m_ints.toCopy);
    declareProperty("uintsToCopy", m_uints.toCopy);
    declareProperty("ulongsToCopy", m_ulongs.toCopy);
    declareProperty("charsToCopy", m_chars.toCopy);
    declareProperty("iparticlesToCopy", m_iparticles.toCopy);
  }

  StatusCode JetMatcherAlg::initialize() {
    std::vector<std::string> sources;
    for (const auto& key: m_sourceJets) {
      sources.emplace_back(key.key());
    }
    std::string target = m_targetJet.key();
    ATH_CHECK(m_floats.initialize(this, sources, target));
    ATH_CHECK(m_doubles.initialize(this, sources, target));
    ATH_CHECK(m_ints.initialize(this, sources, target));
    ATH_CHECK(m_uints.initialize(this, sources, target));
    ATH_CHECK(m_ulongs.initialize(this, sources, target));
    ATH_CHECK(m_chars.initialize(this, sources, target));
    ATH_CHECK(m_iparticles.initialize(this, sources, target));
    ATH_CHECK(m_targetJet.initialize());
    ATH_CHECK(m_sourceJets.initialize());
    ATH_CHECK(m_drDecorator.initialize());
    ATH_CHECK(m_dEtaDecorator.initialize());
    ATH_CHECK(m_dPhiDecorator.initialize());
    ATH_CHECK(m_dPtDecorator.initialize());
    ATH_CHECK(m_matchDecorator.initialize());
    ATH_CHECK(m_nMatchDecoragor.initialize());
    if (!m_linkDecorator.empty()) {
      ATH_CHECK(m_linkDecorator.initialize());
    }
    // choose jet selection option
    using Part = xAOD::IParticle;
    float ptMin = m_sourceMinimumPt.value();
    if (float drMax = m_ptPriorityWithDeltaR.value(); drMax > 0) {
      m_jetSelector = [drMax, ptMin](const Part* tj, const JV& sv) -> Match {
        std::vector<std::pair<float, const Part*>> jets;
        for (const auto* sj: sv) {
          // Dont match if its the same jet - this only happens if
          // matching to the same jc which we only do if trying to
          // find things such as distance to nearest jet
          if (sj == tj) continue;
          if (sj->pt() < ptMin) continue;
          if (tj->p4().DeltaR(sj->p4()) < drMax) {
            jets.emplace_back(sj->pt(), sj);
          }
        }
        auto sItr = std::ranges::max_element(jets);
        if (sItr == jets.end()) return {jets.size(), nullptr};
        return {jets.size(), sItr->second};
      };
    } else {
      m_jetSelector = [ptMin](const Part* tj, const JV& sv) -> Match {
        std::vector<std::pair<float, const Part*>> jets;
        for (const auto* sj: sv) {
          if(sj == tj) continue;
          if ( sj->pt() > ptMin) {
            jets.emplace_back(tj->p4().DeltaR(sj->p4()), sj);
          }
        }
        auto sItr = std::min_element(jets.begin(), jets.end());
        if (sItr == jets.end()) return {jets.size(), nullptr};
        return {jets.size(), sItr->second};
      };
    }
    return StatusCode::SUCCESS;
  }

  namespace {
    using JC = xAOD::IParticleContainer;
    auto descending_pt = [](auto* j1, auto* j2){
      return j1->pt() > j2->pt();
    };
    std::vector<const xAOD::IParticle*> getJetVector(
      SG::ReadHandle<JC>& handle) {
      std::vector<const xAOD::IParticle*> jets(handle->begin(), handle->end());
      std::sort(jets.begin(),jets.end(), descending_pt);
      return jets;
    }
  }

  StatusCode JetMatcherAlg::execute(const EventContext& cxt) const {
    SG::ReadHandle<JC> targetJetGet(m_targetJet, cxt);
    SG::WriteDecorHandle<JC,float> drDecorator(m_drDecorator, cxt);
    SG::WriteDecorHandle<JC,float> detaDecorator(m_dEtaDecorator, cxt);
    SG::WriteDecorHandle<JC,float> dphiDecorator(m_dPhiDecorator, cxt);
    SG::WriteDecorHandle<JC,float> dPtDecorator(m_dPtDecorator, cxt);
    SG::WriteDecorHandle<JC,char> matchDecorator(m_matchDecorator, cxt);
    SG::WriteDecorHandle<JC,unsigned> nMatchDecorator(m_nMatchDecoragor, cxt);
    std::optional<SG::WriteDecorHandle<JC,IPLV>> linkDecorator;
    if (!m_linkDecorator.empty()) linkDecorator.emplace(m_linkDecorator, cxt);
    auto targetJets = getJetVector(targetJetGet);
    std::vector<const xAOD::IParticle*> sourceJets;
    std::map<const xAOD::IParticle*, const xAOD::IParticleContainer*> p2c;
    for (const auto& key: m_sourceJets) {
      const auto* cont = SG::ReadHandle<JC>(key, cxt).cptr();
      sourceJets.insert(sourceJets.end(), cont->begin(), cont->end());
      for (const auto* part: *cont) p2c[part] = cont;
    }
    std::sort(sourceJets.begin(), sourceJets.end(), descending_pt);
    std::vector<MatchedPair<JC>> matches;
    for (const xAOD::IParticle* target: targetJets) {
      const auto [n_matches, source] = m_jetSelector(target, sourceJets);
      nMatchDecorator(*target) = n_matches;
      if (source) {
        drDecorator(*target) = source->p4().DeltaR(target->p4());
        detaDecorator(*target) = source->eta() - target->eta();
        dphiDecorator(*target) = source->p4().DeltaPhi(target->p4());
        dPtDecorator(*target) = source->pt() - target->pt();
        matchDecorator(*target) = 1;
        matches.push_back({source, target});
        if (linkDecorator) linkDecorator.value()(*target) = {
            {*p2c.at(source), source->index(), cxt}
          };
      } else {
        drDecorator(*target) = NAN;
        detaDecorator(*target) = NAN;
        dphiDecorator(*target) = NAN;
        dPtDecorator(*target) = NAN;
        matchDecorator(*target) = 0;
        matches.push_back({nullptr, target});
        if (linkDecorator) linkDecorator.value()(*target) = {};
      }
    }
    m_floats.copy(matches, cxt);
    m_doubles.copy(matches, cxt);
    m_ints.copy(matches, cxt);
    m_uints.copy(matches, cxt);
    m_ulongs.copy(matches, cxt);
    m_chars.copy(matches, cxt);
    m_iparticles.copy(matches, cxt);

    return StatusCode::SUCCESS;
  }
  StatusCode JetMatcherAlg::finalize () {
    return StatusCode::SUCCESS;
  }

}// end namespace ftag
