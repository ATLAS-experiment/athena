/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// Framework includes
#include "MuonRecoChainTester.h"

#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonTesterTree/TrackChi2Branch.h"
#include "MuonPRDTest/SegmentVariables.h"
#include "MuonPRDTest/ParticleVariables.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandle.h"

using namespace MuonVal;
using namespace MuonPRDTest;


namespace {
template <class RefPartType, class SearchPartType>
      const SearchPartType* findClosestParticle(const RefPartType* reference,
                                                const DataVector<SearchPartType>& candidateContainer) {
          const SearchPartType* best{nullptr};
          for (const SearchPartType* candidate : candidateContainer) {
              if (!best || xAOD::P4Helpers::deltaR2(reference, candidate) <
                           xAOD::P4Helpers::deltaR2(reference, best)) {
                  best = candidate;
              }
          }
          return best;            
      }

}
namespace MuonValR4{

    MuonRecoChainTester::MuonRecoChainTester(const std::string& name, ISvcLocator* pSvcLocator):
        AthHistogramAlgorithm{name, pSvcLocator}{}

    StatusCode MuonRecoChainTester::initialize() {
        int evOpts{0};
        if (m_isMC) evOpts |= EventInfoBranch::isMC;
        m_tree.addBranch(std::make_shared<EventInfoBranch>(m_tree, evOpts));

        m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_legacySegmentKey, "LegacySegments", msgLevel()));
        m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_r4PatternSegmentKey, "HoughSegments", msgLevel()));
        ATH_CHECK(m_legacyTrackKey.initialize());
        ATH_CHECK(m_r4TrackKey.initialize());
        ATH_CHECK(m_truthKey.initialize(m_isMC));

        m_legacyTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "LegacyMSTrks");
        m_legacyTrks->addVariable(std::make_shared<TrackChi2Branch>(*m_legacyTrks));
        m_r4Trks = std::make_shared<IParticleFourMomBranch>(m_tree, "HoughMSTrks");
        m_r4Trks->addVariable(std::make_shared<TrackChi2Branch>(*m_r4Trks));
        m_tree.addBranch(m_legacyTrks);
        m_tree.addBranch(m_r4Trks);
        
        if (m_isMC) {
            m_truthTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "TruthMuons");
            m_truthTrks->addVariable<int>("legacyMatched");
            m_truthTrks->addVariable<int>("houghMatched");
            m_tree.addBranch(m_truthTrks);
        }
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonRecoChainTester::execute() {
    
      static const SG::Decorator<int> acc_legacyMatched{"legacyMatched"};
      static const SG::Decorator<int> acc_houghMatched{"houghMatched"};

      const EventContext& ctx{Gaudi::Hive::currentContext()};
      SG::ReadHandle<xAOD::TrackParticleContainer> legacyTrks{m_legacyTrackKey, ctx};
      ATH_CHECK(legacyTrks.isPresent());
      SG::ReadHandle<xAOD::TrackParticleContainer> r4Trks{m_r4TrackKey, ctx};
      ATH_CHECK(r4Trks.isPresent());
      
      for (const xAOD::TrackParticle* trk : *legacyTrks) {
          m_legacyTrks->push_back(trk);
      }
      for (const xAOD::TrackParticle* trk : *r4Trks) {
          m_r4Trks->push_back(trk);
      }
      
      
      ConstDataVector<xAOD::TruthParticleContainer> truthParts{SG::VIEW_ELEMENTS};
      if (!m_truthKey.empty()) {
          SG::ReadHandle<xAOD::TruthParticleContainer> readHandle{m_truthKey, ctx};
          ATH_CHECK(readHandle.isPresent());
          for (const xAOD::TruthParticle* truth : *readHandle) {
              if (!truth->isMuon()) continue;
              if (truth->status() != 1) continue;
              truthParts.push_back(truth);
          }
      }
      for (const xAOD::TruthParticle* truth : truthParts){
          const xAOD::TrackParticle* closestLeg = findClosestParticle(truth, *legacyTrks);
          const xAOD::TrackParticle* closestR4 = findClosestParticle(truth, *r4Trks);
          
          acc_legacyMatched(*truth) = closestLeg && xAOD::P4Helpers::deltaR(truth, closestLeg) ?
                                         static_cast<int>(m_legacyTrks->find(closestLeg)) : -1;
          acc_houghMatched(*truth) = closestR4 && xAOD::P4Helpers::deltaR(truth, closestR4) ?
                                         static_cast<int>(m_r4Trks->find(closestR4)) : -1;

          m_truthTrks->push_back(truth);
      }
      
      
      if(!m_tree.fill(ctx)) {
          return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    }
    StatusCode MuonRecoChainTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}
